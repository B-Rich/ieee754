#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// 'width' is the minimum (zero-padded) width in printed characters
template<typename T>
static void print_bin(T num, unsigned width = CHAR_BIT*sizeof(T)) {
    unsigned const     N_BITS = CHAR_BIT*sizeof(T);
    unsigned long long bit_mask;

    // Skip high zeroes until 'width' is reached
    for (bit_mask = 1LL << (N_BITS - 1);
         bit_mask != (1LL << (width - 1)) && !(num & bit_mask);
         bit_mask >>= 1);

    for (; bit_mask; bit_mask >>= 1)
        putchar((num & bit_mask) ? '1' : '0');
}

// Could parameterize decode_float() and decode_double() on field lengths and
// merge them, but keep things simple and readable for reference

void decode_float(float arg) {
    uint32_t bits = *(uint32_t*)&arg; // Not strict-aliasing safe

    unsigned const FRAC_MASK = 0x7FFFFF;
    double   const FRAC_DIV  = FRAC_MASK + 1;

    unsigned sign     =  bits >> 31;
    unsigned exponent = (bits >> 23) & 0xFF;
    unsigned fraction =  bits & FRAC_MASK;

    enum { INF_MODE, NAN_MODE, SUBNORMAL_MODE, NORMAL_MODE } mode;

    switch (exponent) {
    case 0   : mode = SUBNORMAL_MODE;                        break;
    case 0xFF: mode = (fraction == 0) ? INF_MODE : NAN_MODE; break;
    default  : mode = NORMAL_MODE;                           break;
    }


    fputs("Representation: ", stdout);

    print_bin(sign, 1);
    printf(mode == NAN_MODE ? " (%c, ignored) " : " (%c) ", sign ? '-' : '+');

    print_bin(exponent, 8);
    switch (mode) {
    case NORMAL_MODE:
        printf(" (%u - 127 = %d) ", exponent, (int)exponent - 127);
        break;

    case SUBNORMAL_MODE:
        fputs(" (-126, subnormal) ", stdout);
        break;

    case INF_MODE:
        fputs(" (inf) ", stdout);
        break;

    case NAN_MODE:
        fputs(" (nan) ", stdout);
        break;
    }

    print_bin(fraction, 23);
    switch (mode) {
    case NORMAL_MODE:
        printf(" (%.10f)\n", 1.0 + fraction/FRAC_DIV);
        break;

    case SUBNORMAL_MODE:
        printf(" (%.10f)\n", fraction/FRAC_DIV);
        break;

    case INF_MODE:
        puts(" (inf) ");
        break;

    case NAN_MODE:
        puts(" (nan, precise value ignored) ");
        break;
    }


    fputs("Interpretation: ", stdout);

    switch (mode) {
    case NORMAL_MODE:
        printf("%c%.10f * 2^%d\n", sign ? '-' : '+', 1.0 + fraction/FRAC_DIV, (int)exponent - 127);
        break;

    case SUBNORMAL_MODE:
        printf("%c%.10f * 2^-126 (subnormal)\n", sign ? '-' : '+', fraction/FRAC_DIV);
        break;

    case INF_MODE:
        printf("%cinf\n", sign ? '-' : '+');
        break;

    case NAN_MODE:
        puts("nan");
    }

    printf("Hex           : 0x%04"PRIX32"\n", bits);
}

void decode_double(double arg) {
    uint64_t bits = *(uint64_t*)&arg; // Not strict-aliasing safe

    unsigned long long const FRAC_MASK = 0xFFFFFFFFFFFFFLL;
    double             const FRAC_DIV  = FRAC_MASK + 1;

    unsigned  sign     =  bits >> 63;
    unsigned  exponent = (bits >> 52) & 0x7FF;
    long long fraction =  bits & FRAC_MASK;

    enum { INF_MODE, NAN_MODE, SUBNORMAL_MODE, NORMAL_MODE } mode;

    switch (exponent) {
    case 0    : mode = SUBNORMAL_MODE;                        break;
    case 0x7FF: mode = (fraction == 0) ? INF_MODE : NAN_MODE; break;
    default   : mode = NORMAL_MODE;                           break;
    }


    fputs("Representation: ", stdout);

    print_bin(sign, 1);
    printf(mode == NAN_MODE ? " (%c, ignored) " : " (%c) ", sign ? '-' : '+');

    print_bin(exponent, 11);
    switch (mode) {
    case NORMAL_MODE:
        printf(" (%u - 1023 = %d) ", exponent, (int)exponent - 1023);
        break;

    case SUBNORMAL_MODE:
        fputs(" (-1022, subnormal) ", stdout);
        break;

    case INF_MODE:
        fputs(" (inf) ", stdout);
        break;

    case NAN_MODE:
        fputs(" (nan) ", stdout);
        break;
    }

    print_bin(fraction, 52);
    switch (mode) {
    case NORMAL_MODE:
        printf(" (%.10f)\n", 1.0 + fraction/FRAC_DIV);
        break;

    case SUBNORMAL_MODE:
        printf(" (%.10f)\n", fraction/FRAC_DIV);
        break;

    case INF_MODE:
        puts(" (inf) ");
        break;

    case NAN_MODE:
        puts(" (nan, precise value ignored) ");
        break;
    }


    fputs("Interpretation: ", stdout);

    switch (mode) {
    case NORMAL_MODE:
        printf("%c%.10f * 2^%d\n", sign ? '-' : '+', 1.0 + fraction/FRAC_DIV, (int)exponent - 1023);
        break;

    case SUBNORMAL_MODE:
        printf("%c%.10f * 2^-1022 (subnormal)\n", sign ? '-' : '+', fraction/FRAC_DIV);
        break;

    case INF_MODE:
        printf("%cinf\n", sign ? '-' : '+');
        break;

    case NAN_MODE:
        puts("nan");
    }

    printf("Hex           : 0x%08"PRIX64"\n", bits);
}

static void fail_with_usage() {
    fputs("float argument required\n", stderr);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        fail_with_usage();

    char *endptr;
    double arg = strtod(argv[1], &endptr);
    if (arg == 0 && endptr == argv[1])
        fail_with_usage();

    printf("%f as float:\n", arg);
    decode_float(arg);
    printf("\n%f as double:\n", arg);
    decode_double(arg);
}
