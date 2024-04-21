#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>


/* isinf & isnan macros should be provided by math.h */
#ifndef isinf
/* The value of Inf - Inf is NaN */
#define isinf(n) (isnan((n) - (n)) && !isnan(n))
#endif

#ifndef isnan
/* NaN is the only floating point value that does NOT equal itself.
 * Therefore if n != n, then it is NaN. */
#define isnan(n) ((n != n) ? 1 : 0)
#endif

#define MAX_BUFFER_PRINTF_LEN   (1024)
uint8_t print_ring_buf[MAX_BUFFER_PRINTF_LEN];

#define MAX_PRINT_STR_LEN       (128)
uint32_t buffer_printf_idx = 0;
char* buffer_printf = (char*)print_ring_buf;
uint32_t buffer_printf_len = MAX_BUFFER_PRINTF_LEN;

static uint8_t _putc(uint8_t ch)
{
    if (buffer_printf_idx < (buffer_printf_len - 1)) {
        buffer_printf[buffer_printf_idx] = ch;
        buffer_printf_idx++;
        return 1;
    } else if (buffer_printf_idx == (buffer_printf_len - 1)) {
        buffer_printf[buffer_printf_idx] = 0;
        buffer_printf_idx++;
    }
    return 0;
}

static uint8_t _puts(const char* s)
{
    uint8_t rtn;
    while (*s) {
        rtn = _putc(*s++);
        if (rtn == 0) {
            return 0;
        }
    }
    return 1;
}


#define MAX_UINT_DIGITS     10  /* 2^32 = 4294967296, which has 10 digits */

static int32_t _int_to_str(uint32_t val, char* str, uint8_t len)
{
    const uint32_t g_uart_num_to_dec[MAX_UINT_DIGITS] = {
                                    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
                                    };
    uint32_t quotient;
    uint8_t Divider = MAX_UINT_DIGITS - 1;
    bool FirstDigit = false;

    if (len <= sizeof('\0')) return -1;
    if (str == NULL) return -1;

    *str = '0';
    str[1] = '\0';
    if (val == 0) return 0;
    len--;
    do {
        quotient = val / g_uart_num_to_dec[Divider];
        if (quotient)
            FirstDigit = true;
        if (FirstDigit) {
            *str = '0' + (char)quotient;
            if ((len--) == 0) {    /* if not enough available characters in the string, then... */
                *str = '\0';
                return -1;
            }
            str++;
            val %= g_uart_num_to_dec[Divider];
        }
    } while (Divider--);
    *str = '\0';
    
    return 0;
}


void _buf_printf(const char *fmt, va_list args)
{
    uint32_t j;
    int8_t *p_buf, *p_next;
    uint32_t dword, temp, ch, flag, sftBits;
    char buffer[MAX_UINT_DIGITS + sizeof((char)'\0') + sizeof((char)'-')];
    int32_t len_modifier = 0;
    int32_t zero_fill = 0;

    p_buf = (int8_t *)fmt;

    do {
        if (*p_buf == '%') {
            zero_fill = 0;
            len_modifier = 0;
            p_next = p_buf + 1;
            if (*p_next == '0') {
                zero_fill = 1;
                p_next++;
                p_buf++;
            }
            if ('0' < *p_next && *p_next <= '8') {
                len_modifier = *p_next - '0';
                p_next++;
                p_buf++;
            }
            if ((*p_next == 'x') || (*p_next == 'X')) {
                int8_t hex_start = *p_next - ('x' - 'a');

                dword = va_arg(args, unsigned int);    /* p_para[num]; */
                flag = 0;
                sftBits = 28;
                for (j = 0; j < 8; j++) {
                    temp = (dword&(0xF << sftBits )) >> sftBits;

                    if (temp >= 10)
                        ch = temp - 10 + hex_start;
                    else
                        ch = temp + '0';

                    if (temp || flag || j == 7 || len_modifier >= 8 - j || 
                        (temp == 0 && zero_fill && len_modifier >= 8 -j)) {
                        if (!flag && temp == 0 && j != 7 && !zero_fill)
                            ch = ' ';
                        else
                            flag++;
                        _putc((uint8_t)ch);
                    }

                    sftBits -= 4;
                }
            } else if ((*p_next == 'd') || (*p_next == 'D')) {
                int32_t Val = va_arg(args, unsigned int);    /**(int32_t)p_para[num];**/
                uint32_t uVal;
                if (Val < 0) {
                    uVal = (uint32_t)(-Val);
                    buffer[0] = '-';
                    if (_int_to_str(uVal, &buffer[1], sizeof(buffer) - 1) == 0)
                        _puts(buffer);
                } else {
                    uVal = (uint32_t) Val;
                    if (_int_to_str(uVal, buffer, sizeof(buffer)) == 0)
                        _puts(buffer);
                }
            } else if ((*p_next == 'u') || (*p_next == 'U')) {
                uint32_t val = va_arg(args, unsigned int);
                if (_int_to_str(val, buffer, sizeof(buffer)) == 0)
                    _puts(buffer);
            } else if ((*p_next == 'f') || (*p_next == 'g')) {
                int    sig_digits = 9;
                int    min_scaled = 100000000;
                int    max_scaled = 1000000000;
                double number     = va_arg(args, double);

                /* print minus sign (does not handle negative zero) */
                if (number < 0.0f) {
                    _putc('-');
                    number = -number;
                }

                /* handle zero, NaN, and +/- infinity */
                if (number == 0.0f) {
                    _puts("0");
                } else if (isnan(number)) {
                    _puts("nan");
                } else if (isinf(number)) {
                    _puts("inf");
                } else {
                    int n_int = 0, n;
                    int exponent = 0;
                    int decimals, digits;
                    char buf[16] = {0};
                    /*
					* Scale up or down by powers of 10.  To minimize rounding error,
					* start with a factor/divisor of 10^10, which is the largest
					* power of 10 that can be represented exactly.  Finally, compute
					* (exactly) the remaining power of 10 and perform one more
					* multiplication or division.
					*/
                    if (number < 1.0f) {
                        double factor = 1.0f;
                        while (number < (double)max_scaled / 1e10f) {
                            number *= 1e10f;
                            exponent -= 10;
                        }
                        while (number * factor < (double)min_scaled) {
                            factor *= 10.0f;
                            exponent--;
                        }
                        number *= factor;
                    } else if (number > (double)max_scaled) {
                        double divisor = 1.0f;
                        while (number > (double)min_scaled * 1e10f) {
                            number /= 1e10f;
                            exponent += 10;
                        }
                        while (number / divisor > (double)max_scaled) {
                            divisor *= 10.0f;
                            exponent++;
                        }
                        number /= divisor;
                    } else {
                        /*
						* In this range, we can split off the integer part before
						* doing any multiplications.  This reduces rounding error by
						* freeing up significant bits in the fractional part.
						*/
                        double factor = 1.0f;
                        n_int = (int)number;
                        number -= (double)n_int;
                        while (n_int < min_scaled) {
                            n_int *= 10;
                            factor *= 10.0f;
                            exponent--;
                        }
                        number *= factor;
                    }
                    /* round to nearest integer */
                    n = ((int)(number + number) + 1) / 2;
                    /* round to even if exactly between two integers */
                    if ((n & 1) && (((double)n - number) == 0.5f)) {
                        n--;
                    }
                    n += n_int;
                    if (n >= max_scaled) {
                        n = min_scaled;
                        exponent++;
                    }
                    /* determine where to place decimal point */
                    decimals = ((exponent <= 0) && (exponent >= -(sig_digits + 3))) ? (-exponent) : (sig_digits - 1);
                    exponent += decimals;
                    /* truncate trailing zeroes after decimal point */
                    while ((decimals > 0) && ((n % 10) == 0)) {
                        n /= 10;
                        decimals--;
                    }
                    /* build up buffer in reverse order */
                    digits = 0;
                    while ((n != 0) || (digits < (decimals + 1))) {
                        buf[digits++] = (char)('0' + n % 10);
                        n /= 10;
                    }
                    while (digits > 0) {
                        if (digits == decimals) {
                            _putc('.');
                        }
                        _putc(buf[--digits]);
                    }
                    /* print exponent if needed */
                    if (exponent != 0) {
                        _putc('e');
                        if (exponent < 0) {
                            _putc('-');
                            exponent = -exponent;
                        } else {
                            _putc('+');
                        }
                        digits = 0;
                        while ((exponent != 0) || (digits < 2)) {
                            buf[digits++] = (char)('0' + exponent % 10);
                            exponent /= 10;
                        }
                        while (digits > 0) {
                            _putc(buf[--digits]);
                        }
                    }
                }
            } else if ((*p_next == 's') ||(*p_next == 'S'))
                _puts(va_arg(args, char *));
            else if ((*p_next == 'c') || (*p_next == 'C'))
                _putc(va_arg(args, int));
            else if (*p_next == '%')
                _putc('%');
            else {
                p_buf -= 2;
                _putc('%');
            }

            p_buf += 2;
            continue;
        }

        _putc((uint8_t)*p_buf);

        p_buf++;
    } while (*p_buf);
}

void set_print_buf(void* buf, uint32_t len)
{
    buffer_printf = (char*)buf;
    buffer_printf_len = len;
    buffer_printf_idx = 0;
}

void tiny_printf(const char *fmt, ...)
{
    uint32_t idx = buffer_printf_idx;
    va_list args;
    va_start(args, fmt);
    _buf_printf(fmt, args);
    va_end(args);

    buffer_printf[buffer_printf_idx] = 0;
    if (buffer_printf_idx < buffer_printf_len) {
        buffer_printf_idx++;
    }
    // NcciPrintfReqArg_t arg;
    // arg.buf = (char*)(&buffer_printf[idx]);
    // arg.len = buffer_printf_idx - idx;
    // uint32_t attri = TASK_ATTRI_NEED_RSP;
    // NpuReq(NPU_SEND_PRINTF, attri, 0, (void*)(&arg), sizeof(NcciPrintfReqArg_t));
    // if (buffer_printf_idx > (buffer_printf_len - MAX_PRINT_STR_LEN)) {
    //     buffer_printf_idx = 0;
    // }
}

