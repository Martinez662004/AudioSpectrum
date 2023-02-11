/*#include <SDL2/SDL.h>
#include <fftw3.h>
#include <iostream>

constexpr auto SAMPLE_RATE = 44100;
const int NUM_SAMPLES = 2048;
const int NUM_BARS = NUM_SAMPLES / 44;
const int AVG_SAMPLES = 32;
float freq;


struct WavData {
    SDL_AudioSpec spec;
    Uint8* buffer;
    Uint32 length;
    Uint8* position;
    Uint32 played;
};

fftw_plan plan;
fftw_complex* in, * out;

void audio_callback(void* userdata, Uint8* stream, int len) {
    WavData* wavData = (WavData*)userdata;
    Uint32 remaining = wavData->length - wavData->played;
    if (remaining < len) {
        memcpy(stream, wavData->position, remaining);
        memcpy(stream + remaining, wavData->buffer, len - remaining);
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
    WavData wavData;
    if (SDL_LoadWAV("d.wav", &wavData.spec, &wavData.buffer, &wavData.length) == NULL) {
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
    SDL_Window* window = SDL_CreateWindow("Finalmente", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, NUM_BARS * 10, 300, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, NUM_BARS * 10, 300);

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
            if (bufferPos >= wavData.length) {
                bufferPos = 0;
            }
            in[i][0] = wavData.buffer[bufferPos++] / 128.0;
            in[i][1] = 0.0;
        }

        // Aplica la transformada de Fourier
        fftw_execute(plan);


        // Dibuja las barras
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < NUM_BARS; i++) {
            int height = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) / NUM_SAMPLES * 300;
            freq = i * ((double)SAMPLE_RATE / NUM_SAMPLES);

            SDL_Rect rect = { i * 10, 300 - height, 8, height };
            SDL_RenderFillRect(renderer, &rect);
        }

        std::cout << sqrt(out[3][0] * out[3][0] + out[3][1] * out[3][1]) / NUM_SAMPLES * 300 << std::endl;
        SDL_RenderPresent(renderer);
        SDL_Delay(16.66);
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
}*/