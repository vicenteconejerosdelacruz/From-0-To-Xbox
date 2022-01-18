#pragma once

#include <wrl.h>

namespace DX
{
	// Clase de asistente para controlar el tiempo de las animaciones y simulaciones.
	class StepTimer
	{
	public:
		StepTimer() : 
			m_elapsedTicks(0),
			m_totalTicks(0),
			m_leftOverTicks(0),
			m_frameCount(0),
			m_framesPerSecond(0),
			m_framesThisSecond(0),
			m_qpcSecondCounter(0),
			m_isFixedTimeStep(false),
			m_targetElapsedTicks(TicksPerSecond / 60)
		{
			if (!QueryPerformanceFrequency(&m_qpcFrequency))
			{
				throw ref new Platform::FailureException();
			}

			if (!QueryPerformanceCounter(&m_qpcLastTime))
			{
				throw ref new Platform::FailureException();
			}

			// Inicializar delta máximo en una décima parte de un segundo.
			m_qpcMaxDelta = m_qpcFrequency.QuadPart / 10;
		}

		// Obtener el tiempo transcurrido desde la llamada a Update anterior.
		uint64 GetElapsedTicks() const						{ return m_elapsedTicks; }
		double GetElapsedSeconds() const					{ return TicksToSeconds(m_elapsedTicks); }

		// Obtener el tiempo total desde el inicio del programa.
		uint64 GetTotalTicks() const						{ return m_totalTicks; }
		double GetTotalSeconds() const						{ return TicksToSeconds(m_totalTicks); }

		// Obtener el número total de actualizaciones desde el inicio del programa.
		uint32 GetFrameCount() const						{ return m_frameCount; }

		// Obtener el valor de framerate actual.
		uint32 GetFramesPerSecond() const					{ return m_framesPerSecond; }

		// Configurar si se va a usar el modo de timestep fijo o variable.
		void SetFixedTimeStep(bool isFixedTimestep)			{ m_isFixedTimeStep = isFixedTimestep; }

		// Configurar la frecuencia con la que se llama a Update cuando se usa el modo de timestep fijo.
		void SetTargetElapsedTicks(uint64 targetElapsed)	{ m_targetElapsedTicks = targetElapsed; }
		void SetTargetElapsedSeconds(double targetElapsed)	{ m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

		// Formato de entero que representa la hora en 10.000.000 pasos por segundo.
		static const uint64 TicksPerSecond = 10000000;

		static double TicksToSeconds(uint64 ticks)			{ return static_cast<double>(ticks) / TicksPerSecond; }
		static uint64 SecondsToTicks(double seconds)		{ return static_cast<uint64>(seconds * TicksPerSecond); }

		// Después de una interrupción temporal intencionada (por ejemplo, una operación de E/S de bloqueo)
		// se llama a esto para evitar que la lógica de timestep fijo intente una serie de
		// llamadas de recuperación a Update.

		void ResetElapsedTime()
		{
			if (!QueryPerformanceCounter(&m_qpcLastTime))
			{
				throw ref new Platform::FailureException();
			}

			m_leftOverTicks = 0;
			m_framesPerSecond = 0;
			m_framesThisSecond = 0;
			m_qpcSecondCounter = 0;
		}

		// Actualizar el estado del temporizador llamando a la función Update especificada el número de veces que sea necesario.
		template<typename TUpdate>
		void Tick(const TUpdate& update)
		{
			// Consultar la hora actual.
			LARGE_INTEGER currentTime;

			if (!QueryPerformanceCounter(&currentTime))
			{
				throw ref new Platform::FailureException();
			}

			uint64 timeDelta = currentTime.QuadPart - m_qpcLastTime.QuadPart;

			m_qpcLastTime = currentTime;
			m_qpcSecondCounter += timeDelta;

			// Fijar los deltas de tiempo excesivamente largos (p. ej., tras una pausa del depurador).
			if (timeDelta > m_qpcMaxDelta)
			{
				timeDelta = m_qpcMaxDelta;
			}

			// Convertir las unidades QPC en un formato de marca de graduación canónico. Este no puede desbordarse debido al bloqueo anterior.
			timeDelta *= TicksPerSecond;
			timeDelta /= m_qpcFrequency.QuadPart;

			uint32 lastFrameCount = m_frameCount;

			if (m_isFixedTimeStep)
			{
				// Lógica de actualización de timestep fijo

				// Si la aplicación se ejecuta muy cerca del tiempo transcurrido de destino (en 1/4 de milisegundo), se bloqueará
				// el reloj para que coincida exactamente con el valor de destino. Esto impide que se acumulen con el tiempo
				// errores pequeños e irrelevantes. Sin este bloqueo, un juego que solicitara una actualización fija
				// con un valor de fps de 60, ejecutándose con vsync habilitado en una pantalla 59.94 NTSC, acabaría
				// acumulando tantos pequeños errores que borraría el marco. Es mejor redondear estas 
				// pequeñas desviaciones a cero para dejar que todo siga su curso.

				if (abs(static_cast<int64>(timeDelta - m_targetElapsedTicks)) < TicksPerSecond / 4000)
				{
					timeDelta = m_targetElapsedTicks;
				}

				m_leftOverTicks += timeDelta;

				while (m_leftOverTicks >= m_targetElapsedTicks)
				{
					m_elapsedTicks = m_targetElapsedTicks;
					m_totalTicks += m_targetElapsedTicks;
					m_leftOverTicks -= m_targetElapsedTicks;
					m_frameCount++;

					update();
				}
			}
			else
			{
				// Lógica de actualización de timestep variable.
				m_elapsedTicks = timeDelta;
				m_totalTicks += timeDelta;
				m_leftOverTicks = 0;
				m_frameCount++;

				update();
			}

			// Controlar el valor de framerate actual.
			if (m_frameCount != lastFrameCount)
			{
				m_framesThisSecond++;
			}

			if (m_qpcSecondCounter >= static_cast<uint64>(m_qpcFrequency.QuadPart))
			{
				m_framesPerSecond = m_framesThisSecond;
				m_framesThisSecond = 0;
				m_qpcSecondCounter %= m_qpcFrequency.QuadPart;
			}
		}

	private:
		// Los datos de tiempo de origen usan unidades QPC.
		LARGE_INTEGER m_qpcFrequency;
		LARGE_INTEGER m_qpcLastTime;
		uint64 m_qpcMaxDelta;

		// Los datos de tiempo derivados usan un formato de marca de graduación canónico.
		uint64 m_elapsedTicks;
		uint64 m_totalTicks;
		uint64 m_leftOverTicks;

		// Miembros para seguimiento del valor de framerate actual.
		uint32 m_frameCount;
		uint32 m_framesPerSecond;
		uint32 m_framesThisSecond;
		uint64 m_qpcSecondCounter;

		// Miembros para configurar el modo fijo de timestep.
		bool m_isFixedTimeStep;
		uint64 m_targetElapsedTicks;
	};
}
