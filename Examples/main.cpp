#include <SDL2/SDL.h>
#include <fftw3.h>

const char* file = "audio.wav";                // Ruta de acceso

constexpr int SAMPLE_RATE = 44100;
constexpr int NUM_SAMPLES = 2048;
constexpr int NUM_BARS = NUM_SAMPLES / 44 ;
constexpr int AVG_SAMPLES = 22;

constexpr int HEIGHT = 400;
constexpr int SIZE_BARS = 8;

constexpr double dampingFactor = 0.8f;          // Suavisado de movimiento
constexpr double threshold = 9.0f;              // Umbral
constexpr double amplificationFactor = 5.9f;    // Amplificador de movimiento

double prevHeights[NUM_BARS] = { 0 };
double avgAmplitudes[NUM_BARS] = { 0 };

float freq;
double hann;
int height;

Uint32 Delay = 1000 / 60;                       // 60 FPS

//Estructura del Audio
struct WavData {
    SDL_AudioSpec spec;
    Uint8* buffer;
    Uint32 length;
    Uint8* position;
    Uint32 played;
};

//Inicia FFT
fftw_plan plan;
fftw_complex* in, * out;

void audio_callback(void* userdata, Uint8* stream, int len) {
    WavData* wavData = (WavData*)userdata;
    Uint32 remaining = wavData->length - wavData->played;
    if ((int)remaining < len) {
        memcpy(stream, wavData->position, remaining);
        memcpy(stream + remaining, wavData->buffer, len - static_cast<size_t>(remaining));
        wavData->position = wavData->buffer;
        wavData->played = len - remaining;
    }
    else {
        memcpy(stream, wavData->position, len);
        wavData->position += len;
        wavData->played += len;
    }
}

int main(int argc, char* argv[]) {
    // Carga el audio
    WavData wavData{};
    if (SDL_LoadWAV(file, &wavData.spec, &wavData.buffer, &wavData.length) == NULL) {
        fprintf(stderr, "Error al cargar el archivo de audio: %s\n", SDL_GetError());
        return 1;
    }
    wavData.played = 0;
    wavData.position = wavData.buffer;

    // Inicializa SDL
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Inicializa el audio
    wavData.spec.callback = audio_callback;
    wavData.spec.userdata = &wavData;
    if (SDL_OpenAudio(&wavData.spec, NULL) < 0) {
        fprintf(stderr, "Error al abrir el audio: %s\n", SDL_GetError());
        return 1;
    }
    SDL_PauseAudio(0);

    // Inicializa la ventana
    SDL_Window* window = SDL_CreateWindow("Audio", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, NUM_BARS * 10, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, NUM_BARS * 10, HEIGHT);

    // Inicializa FFTW
    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES);
    plan = fftw_plan_dft_1d(NUM_SAMPLES, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
    // Dibuja las barras
    int bufferPos = 0;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Copia los muestreos a un buffer
        for (int i = 0; i < NUM_SAMPLES; i++) {
            if ((unsigned)bufferPos >= wavData.length) {
                bufferPos = 0;
            }
            hann = 0.5 * (1 - cos(2 * M_PI * i / NUM_SAMPLES));
            in[i][0] = (wavData.buffer[bufferPos++] / 128.0) * hann;
            in[i][1] = 0.0;
        }

        // Aplica la transformada de Fourier
        fftw_execute(plan);

        // Calcula la amplitud promedio de cada barra
        for (int i = 0; i < NUM_BARS; i++) {
            for (int j = 0; j < AVG_SAMPLES; j++) {
                avgAmplitudes[i] += sqrt(pow(out[i * AVG_SAMPLES + j][0], 2) + pow(out[i * AVG_SAMPLES + j][1], 2));
            }
            avgAmplitudes[i] /= AVG_SAMPLES;
            if (avgAmplitudes[i] > threshold) {
                avgAmplitudes[i] *= amplificationFactor;
            }
        }

        // Dibuja las barras
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (int i = 0; i < NUM_BARS; i++) {
            height = (int)avgAmplitudes[i];
            freq = i * ((float)SAMPLE_RATE / NUM_SAMPLES);

            // Aplica la fórmula de amortiguamiento
            height = (int)((height * (1 - dampingFactor)) + (prevHeights[i] * dampingFactor));
            prevHeights[i] = height;
            SDL_SetRenderDrawColor(renderer, (Uint8)(250*(i/prevHeights[i])) , (Uint8)(0), (Uint8)(prevHeights[i] + (2*(i/ prevHeights[i]))), 255);
            SDL_Rect rect = { i * 10, HEIGHT - height, SIZE_BARS, height };
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(Delay);
    }

    // Limpieza
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    SDL_CloseAudio();
    SDL_FreeWAV(wavData.buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
