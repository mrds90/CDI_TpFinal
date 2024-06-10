import argparse
from datetime import datetime

# Templates
c_template = """
/**
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

// Numerator coefficients in Q15
{num_defines}

// Denominator coefficients in Q15
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

h_template = """
/**
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

# Function to generate content of the .c file
def generate_c_file(source_file, num_defines, den_defines, function_definitions, date, file_upper, reset_function, num_size, den_size):
    return c_template.format(
        source_file=source_file,
        num_defines=num_defines,
        den_defines=den_defines,
        num_size=num_size,
        den_size=den_size,
        function_definitions=function_definitions,
        date=date,
        header_file=source_file,
        file_upper=file_upper,
        reset_function = reset_function
    )

# Function to generate content of the .h file
def generate_h_file(header_file, header_guard, date, file_upper):
    return h_template.format(
  
        header_file=header_file,
        header_guard=header_guard,
        date=date,
        file_upper=file_upper
    )

def generate_reset_function(file_upper):
    reset_function = f"""/**
 * @brief Resets the filter state.
 *
 * This function resets the internal state of the filter buffers.
 */
int32_t {file_upper}_Reset() {{
    memset(output_buffer, 0, sizeof(output_buffer));
    memset(input_buffer, 0, sizeof(input_buffer));
}}"""
    return reset_function

def generate_function_definitions(file_upper, num_coeffs, den_coeffs):
    num_size = len(num_coeffs)
    den_size = len(den_coeffs)
        
    num_defines = "\n".join([f"#define NUM{i} F_TO_Q15({num_coeffs[i]})" for i in range(num_size)])
    den_defines = "\n".join([f"#define DEN{i} F_TO_Q15({den_coeffs[i]})" for i in range(den_size)])  # DEN0 is included now
    
    function_code = f"""/**
 * @brief Filters input signal using a digital filter.
 *
 * This function implements a digital filter to process the input signal. It takes
 * the input signal in Q15 format and applies a numerator-denominator filter with
 * coefficients provided as Q15 fixed-point values.
 *
 * @param input Input signal in Q15 format.
 * @return Filtered output signal in Q15 format.
 */
int32_t {file_upper}_Filter(int32_t input) {{
    // Shift values in the input buffer
    for (int i = NUM_SIZE - 1; i > 0; --i) {{
        input_buffer[i] = input_buffer[i - 1];
    }}
    input_buffer[0] = input;

    // Calculate the numerator part
    int32_t output = 0;
{"".join([f"    output += (NUM{i} * input_buffer[{i}]) >> 15;\n" for i in range(num_size)])}

    // Calculate the denominator part
{"".join([f"    output -= (DEN{i} * output_buffer[{i-1}]) >> 15;\n" for i in range(1, den_size)])}

    // Shift values in the output buffer
    for (int i = DEN_SIZE - 2; i > 0; --i) {{
        output_buffer[i] = output_buffer[i - 1];
    }}
    output_buffer[0] = output;

    return output;
}}"""
    return num_size, den_size, num_defines, den_defines, function_code

def main():
    parser = argparse.ArgumentParser(description="Generate .c and .h files based on templates.")
    parser.add_argument("-file", default="real_world_filter", help="Base name for the .c and .h files")
    parser.add_argument("-num_coeffs", nargs='+', type=float, default=[0.04976845243756167, 0.035050642374672925], help="Numerator coefficients")
    parser.add_argument("-den_coeffs", nargs='+', type=float, default=[1.0, -1.2631799459800208, 0.34799904079225535], help="Denominator coefficients")

    args = parser.parse_args()
    base_name = args.file
    header_file = base_name
    source_file = base_name
    file_upper = base_name.upper()
    header_guard = f"{file_upper}_H"
    date = datetime.now().strftime("%Y-%m-%d")
    
    # Generate function definitions
    num_size, den_size, num_defines, den_defines, function_definitions = generate_function_definitions(file_upper, args.num_coeffs, args.den_coeffs)
    
    # Generate reset function
    reset_function = generate_reset_function(file_upper)
    
    # Generate content of the files
    h_content = generate_h_file(header_file, header_guard, date, file_upper)
    c_content = generate_c_file(source_file, num_defines, den_defines, function_definitions, date, file_upper, reset_function, num_size, den_size)
    
    # Write the files
    with open(f"{base_name}.c", "w") as c_file:
        c_file.write(c_content)
        
    with open(f"{base_name}.h", "w") as h_file:
        h_file.write(h_content)

    print(f"Files {base_name}.c and {base_name}.h generated successfully.")

if __name__ == "__main__":
    main()
