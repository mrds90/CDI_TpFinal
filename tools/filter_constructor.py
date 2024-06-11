import argparse
from datetime import datetime

class FilterGenerator:
    # Class-level constants for templates
    c_template = """/**
 * @file {source_file}.c
 * @author Marcos Dominguez
 *
 * @brief Module description
 *
 * @version X.Y
 * @date {date}
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "{header_file}.h"
#include <string.h>

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define F_TO_Q15(x)  (int32_t)((x) * (1 << 15))

#define NUM_SIZE {num_size}
#define DEN_SIZE {den_size}

/* Numerator coefficients in Q15 */
{num_defines}

/* Denominator coefficients in Q15 */
{den_defines}

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

static int32_t input_buffer[NUM_SIZE] = {{[0 ... (NUM_SIZE - 1)] = 0}};
static int32_t output_buffer[DEN_SIZE - 1] = {{[0 ... (DEN_SIZE - 2)] = 0}};

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

{reset_function}

{function_definitions}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
"""

    h_template = """/**
 * @file {header_file}.h
 * @author Marcos Dominguez
 *
 * @brief Module description
 *
 * @version X.Y
 * @date {date}
 */

#ifndef {header_guard}
#define {header_guard}

/*========= [DEPENDENCIES] =====================================================*/

#include <stdint.h>

/*========= [PUBLIC MACRO AND CONSTANTS] =======================================*/

/*========= [PUBLIC DATA TYPE] =================================================*/

/*========= [PUBLIC FUNCTION DECLARATIONS] =====================================*/

/**
 * @brief Resets the filter state.
 *
 * This function resets the internal state of the filter buffers.
 */
int32_t {file_upper}_Reset();

/**
 * @brief Filters input signal using a digital filter.
 *
 * This function implements a digital filter to process the input signal. It takes
 * the input signal in Q15 format and applies a numerator-denominator filter with
 * coefficients provided as Q15 fixed-point values.
 *
 * @param input Input signal in Q15 format.
 * @return Filtered output signal in Q15 format.
 */
int32_t {file_upper}_Filter(int32_t input);

#endif  /* {header_guard} */
"""

    def __init__(self, base_name, num_coeffs, den_coeffs):
        self.base_name = base_name
        self.num_coeffs = num_coeffs
        self.den_coeffs = den_coeffs
        self.header_file = base_name
        self.source_file = base_name
        self.file_upper = base_name.upper()
        self.header_guard = f"{self.file_upper}_H"
        self.date = datetime.now().strftime("%Y-%m-%d")
        
        self.num_size, self.den_size, self.num_defines, self.den_defines, self.function_definitions = self.generate_function_definitions()
        self.reset_function = self.generate_reset_function()

    def generate_c_file(self):
        return self.c_template.format(
            source_file=self.source_file,
            num_defines=self.num_defines,
            den_defines=self.den_defines,
            num_size=self.num_size,
            den_size=self.den_size,
            function_definitions=self.function_definitions,
            date=self.date,
            header_file=self.header_file,
            file_upper=self.file_upper,
            reset_function=self.reset_function
        )

    def generate_h_file(self):
        return self.h_template.format(
            header_file=self.header_file,
            header_guard=self.header_guard,
            date=self.date,
            file_upper=self.file_upper
        )

    def generate_reset_function(self):
        reset_function = f"""/**
 * @brief Resets the filter state.
 *
 * This function resets the internal state of the filter buffers.
 */
int32_t {self.file_upper}_Reset() {{
    memset(output_buffer, 0, sizeof(output_buffer));
    memset(input_buffer, 0, sizeof(input_buffer));
}}"""
        return reset_function

    def generate_function_definitions(self):
        num_size = len(self.num_coeffs)
        den_size = len(self.den_coeffs)
        
        num_defines = "\n".join([f"#define NUM{i} F_TO_Q15({self.num_coeffs[i]})" for i in range(num_size)])
        den_defines = "\n".join([f"#define DEN{i} F_TO_Q15({self.den_coeffs[i]})" for i in range(den_size)])  # DEN0 is included now
        
        function_code = (f"""/**
 * @brief Filters input signal using a digital filter.
 *
 * This function implements a digital filter to process the input signal. It takes
 * the input signal in Q15 format and applies a numerator-denominator filter with
 * coefficients provided as Q15 fixed-point values.
 *
 * @param input Input signal in Q15 format.
 * @return Filtered output signal in Q15 format.
 */
int32_t {self.file_upper}_Filter(int32_t input) {{
    /* Shift values in the input buffer */
    for (int i = NUM_SIZE - 1; i > 0; --i) {{
        input_buffer[i] = input_buffer[i - 1];
    }}
    input_buffer[0] = input;

    /* Calculate the numerator part */
    int32_t output = 0;
""" +
            "".join([f"    output += (NUM{i} * input_buffer[{i}]) >> 15;\n" for i in range(num_size)]) +
        f"""
    /* Calculate the denominator part */
""" +
            "".join([f"    output -= (DEN{i} * output_buffer[{i-1}]) >> 15;\n" for i in range(1, den_size)]) +
        f"""
    /* Shift values in the output buffer */
    for (int i = DEN_SIZE - 2; i > 0; --i) {{
        output_buffer[i] = output_buffer[i - 1];
    }}
    output_buffer[0] = output;

    return output;
}}""")
        return num_size, den_size, num_defines, den_defines, function_code

    def write_files(self):
        c_content = self.generate_c_file()
        h_content = self.generate_h_file()
        
        with open(f"{self.base_name}.c", "w") as c_file:
            c_file.write(c_content)
            
        with open(f"{self.base_name}.h", "w") as h_file:
            h_file.write(h_content)
            
        print(f"Files {self.base_name}.c and {self.base_name}.h generated successfully.")

def main():
    parser = argparse.ArgumentParser(description="Generate .c and .h files based on templates.")
    parser.add_argument("-file", default="real_world_filter", help="Base name for the .c and .h files")
    parser.add_argument("-num_coeffs", nargs='+', type=float, default=[0.04976845243756167, 0.035050642374672925], help="Numerator coefficients")
    parser.add_argument("-den_coeffs", nargs='+', type=float, default=[1.0, -1.2631799459800208, 0.34799904079225535], help="Denominator coefficients")

    args = parser.parse_args()
    
    generator = FilterGenerator(args.file, args.num_coeffs, args.den_coeffs)
    generator.write_files()

if __name__ == "__main__":
    main()
