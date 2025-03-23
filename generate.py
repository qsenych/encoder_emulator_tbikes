import os
import re

SRC_PATH = os.path.join("Core", "Src")
MAIN_PATH = os.path.join(SRC_PATH, "main.c")

# custom defines for the codegen
GLOBAL_DEFS = {"SIN_PERIODS_PER_REV": 1, "HALL_PERIODS_PER_REV": 20, "OFFSET": 0}


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
    test_array = [1, 2, 3]
    array_to_c(test_array)


if __name__ == "__main__":
    main()
