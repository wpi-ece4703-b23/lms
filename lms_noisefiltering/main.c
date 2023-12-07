#include "xlaudio.h"
#include "xlaudio_armdsp.h"
#include <stdlib.h>

#define BETA 0.01f
#define N 21

float32_t w[N] = {0.0};
float32_t x[N] = {0.0};

float32_t sinlu[16];
unsigned luptr = 0;

void initlu() {
    int i;
    for (i=0; i<16; i++)
        sinlu[i] = 0.05 * sin(2 * PI * i / 16.0);
}

#include <stdio.h>

uint16_t processSample(uint16_t s) {
    int i;

    float32_t noise = sinlu[luptr];
    luptr = (luptr + 1) % 16;

    x[0] = noise;

    float32_t input = xlaudio_adc14_to_f32(s) + noise;

    // compute filter output
    float32_t y = 0.;
    for (i=0; i<N; i++) {
        y += w[i]*x[i];
    }

    // error
    float32_t e = input - y;

    if (xlaudio_pushButtonLeftDown()) {
        // clear coefficients
        for (i=N-1; i >= 0; i--) {
            printf("%f\n", w[i]);
            w[i] = 0.;
        }
    } else {
        // update coefficients
        for (i=N-1; i >= 0; i--)
            w[i] += BETA * e * x[i];
    }

    // shift delay line
    for (i=N-1; i > 0; i--)
        x[i] = x[i-1];

    if (xlaudio_pushButtonRightDown())
        return xlaudio_f32_to_dac14(y);
    else
        return xlaudio_f32_to_dac14(e);
}

int main(void) {
    WDT_A_hold(WDT_A_BASE);

    initlu();

    xlaudio_init_intr(FS_16000_HZ, XLAUDIO_J1_2_IN, processSample);
    xlaudio_run();

    return 1;
}

