import math
import os
import re

import numpy as np

SRC_PATH = os.path.join("Core", "Src")
INCLUDE_PATH = os.path.join("Core", "Inc")
MAIN_PATH = os.path.join(SRC_PATH, "main.c")
LUT_HEADER_PATH = os.path.join(INCLUDE_PATH, "atan_lut.h")
GLOBAL_DEFS = {
    "SIN_PERIODS_PER_REV": 1,
    "HALL_PERIODS_PER_REV": 20,
    "OFFSET": 0,
    "LUT_SIZE": 1024,
    # "ADC_MAX": 4012.855,  # 4.1 * 5.6 / (1.5 + 5.6) / 3.3 * (2 ^ 12 - 1),
    # "ADC_MIN": 880.87,  # 0.9 * 5.6 / (1.5 + 5.6) / 3.3 * (2 ^ 12 - 1),
    "ADC_MAX": 3479.0,  # As tested
    "ADC_MIN": 1850.0,  # As tested
}


def generate_atan2_lut(size):
    lut = np.zeros(size)
    for i in range(size):
        r = i / (size - 1)
        lut[i] = math.atan(r)
    return lut


def save_lut_to_header(lut, filename, size):
    header_content = "#ifndef ATAN2_LUT_H\n"
    header_content += "#define ATAN2_LUT_H\n\n"
    header_content += "// Generated LUT for atan2 with {size} entries\n"
    header_content += f"const float ATAN_LUT[{size}] = {{\n"
    for _, value in enumerate(lut):
        header_content += f"    {value:.8f},\n"
    header_content += "};\n\n"
    header_content += "#endif // ATAN2_LUT_H\n"
    with open(filename, "w") as f:
        f.write(header_content)


def print_lut(lut):
    for i, value in enumerate(lut):
        print(f"Index {i}: {value} radians")


def replace_string(pattern: str, replacement_string: str):
    with open(MAIN_PATH, "r") as file:
        file_content = file.read()
    updated_content = re.sub(
        pattern,
        r"\1" + "\n" + replacement_string + r"\n" + r"\3",
        file_content,
        flags=re.DOTALL,
    )
    with open(MAIN_PATH, "w") as file:
        file.write(updated_content)
    return


def const_to_c(def_dict: dict):
    define_str = ""
    for key, value in def_dict.items():
        define_str += f"#define {key} {value}\n"
    replace_string(
        r"(/\* USER CODE BEGIN PD \*/)(.*?)(/\* USER CODE END PD \*/)", define_str
    )


def array_to_c(array: list):
    array_str = """
        /*\tAutogenerate LTU\t*/
        uint8_t gpio_lut[] = {
        """
    for value in array:
        array_str += str(value) + ","
    array_str.rstrip(",")
    array_str += "};"
    replace_string(
        r"(/\* USER CODE BEGIN 1 \*/)(.*?)(/\* USER CODE END 1 \*/)", array_str
    )
    return


def main():
    const_to_c(GLOBAL_DEFS)
    lut = generate_atan2_lut(GLOBAL_DEFS["LUT_SIZE"])
    save_lut_to_header(lut, LUT_HEADER_PATH, GLOBAL_DEFS["LUT_SIZE"])
    print_lut(lut[:10])


if __name__ == "__main__":
    main()
