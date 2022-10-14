#include <iostream>
#include <bitset>
#include <cmath>

using namespace std;
const string SEPARATOR = "  ";


// Характеристика 4 біти, мантиса - 10 бітів
// 2^8 = 256,  0...255,  127 in the middle => we add 127 to the "real" exponent to cover the case when the exponent is negative
// 2^7 -1


// 2^4 =  16,  0....15,    7 in the middle => we add   7 to the "real" exponent to cover the case when the exponent is negative

class Digit_with_floating_point {
private:
    bool sign_bit;

    int num_of_characteristic_bits;

    //    AKA exponential part
    bool *characteristic_bits;

    bool implicit_bit = 1;

    //  дробная часть
    int num_of_mantissa_bits;
    bool *mantissa_bits;

    string description;

    // Something like 111.01. We need this form to later calculate standardized form  and the exponential part.
    string non_normalised_form;

    string normalised_form;

    // A number of digits we have to shift to achieve normalised form. e.g: 111.01 = 1.1101 * 10^2 => the shift here is 2.
    int shift = 0;

public:
    Digit_with_floating_point(int num_of_characteristic_bits, int num_of_mantissa_bits) {
        this->num_of_characteristic_bits = num_of_characteristic_bits;
        this->characteristic_bits = new bool[num_of_characteristic_bits];

        this->num_of_mantissa_bits = num_of_mantissa_bits;
        this->mantissa_bits = new bool[num_of_mantissa_bits];
    }

    void set_mantissa_bits(string bits) {
        for (int i = 0; i < bits.length(); i++) {
            if (bits[i] == '1') {
                this->mantissa_bits[i] = 1;
            } else if (bits[i] == '0') {
                this->mantissa_bits[i] = 0;
            } else {
                cout << "ERROR: WRONG MANTISSA BITS INITIALIZATION!" << endl;
                exit(1);
            }
        }
    }

    void set_characteristic_bits(string bits) {
        for (int i = 0; i < bits.length(); i++) {
            if (bits[i] == '1') {
                this->characteristic_bits[i] = 1;
            } else if (bits[i] == '0') {
                this->characteristic_bits[i] = 0;
            } else {
                cout << "ERROR: WRONG CHARACTERISTIC BITS INITIALIZATION!" << endl;
                exit(1);
            }
        }
    }

    void set_sign_bit(bool bit) {
        this->sign_bit = bit;
    }

    void set_implicit_bit(bool bit) {
        this->implicit_bit = bit;
    }

    void print_info() {
        cout << sign_bit << SEPARATOR;

        for (int i = 0; i < num_of_characteristic_bits; i++) {
            cout << this->characteristic_bits[i];
        }
        cout << SEPARATOR << implicit_bit << SEPARATOR;
        for (int i = 0; i < num_of_mantissa_bits; i++) {
            cout << this->mantissa_bits[i];
        }
        cout << SEPARATOR << description << endl;
    }

    void set_description(string desc) {
        this->description = desc;
    }

    string to_binary(int n) {
        string r;
        while (n != 0) {
            r = (n % 2 == 0 ?
                 "0" :
                 "1"
                ) + r;
            n /= 2;
        }
        return r;
    }

    string floating_part_to_binary(float n) {
        string result = "";
        float tmp = n;
        while (tmp != 1.00f && result.length() < num_of_mantissa_bits) {
            tmp *= 2;
            result += to_string((tmp >= 1.00f));
            if (tmp > 1) {
                tmp = tmp - (int) tmp;
            }
        }
        return result;
    }

    int calculate_shift(string overall_form) {
        int index_of_a_dot = overall_form.find('.');
        if (index_of_a_dot == -1) {
            cout << "ERROR! PROVIDED WRONG FLOATING FORM!";
            exit(-1);
        }

        // We have to perform a dot shift in a left direction, eg. 111.01 = 1.1101 * 10^2 (shifted left by 2 units)
        int shift = 0;
        if (overall_form[0] == '1') {

            shift = index_of_a_dot - 1;
        }
        // We have to perform a dot shift in a right direction, eg. 0.0011 = 1.1 * 10^(-3) (shifted right by 3 units)
        if (overall_form[0] == '0') {
            int index_of_first_one = overall_form.find_first_of('1');

            // If there are no '1's in the string, we exit the program, because something went wrong.
            if (index_of_first_one == -1) {
                cout << "ERROR! FLOAT CAN'T BE NORMALISED!";
                exit(-1);
            }
            shift = 1 - index_of_first_one;
        }
        return shift;
    }

    string calculate_normalised_form(string overall_form) {
        int index_of_a_dot = overall_form.find('.');
        if (index_of_a_dot == -1) {
            cout << "ERROR! PROVIDED WRONG FLOATING FORM!";
            exit(-1);
        }

        // We have to perform a dot shift in a left direction, eg. 111.01 = 1.1101 * 10^2 (shifted left by 2 units)
        int shift = calculate_shift(overall_form);

        if (overall_form[0] == '1') {
            char tmp = overall_form[1];
            overall_form[1] = '.';
            overall_form[index_of_a_dot] = tmp;
        }

        // We have to perform a dot shift in a right direction, eg. 0.0011 = 1.1 * 10^(-3) (shifted right by 3 units)
        if (overall_form[0] == '0') {
            int index_of_first_one = overall_form.find_first_of('1');
            // If there are no '1's in the string, we exit the program, because something went wrong.
            if (index_of_first_one == -1) {
                cout << "ERROR! FLOAT CAN'T BE NORMALISED!";
                exit(-1);
            }
            string res(overall_form.length() - index_of_first_one + 1, '\0');
            res[0] = overall_form[index_of_first_one];
            res[1] = '.';

            for (int index_in_result = 2; index_in_result < res.length(); index_in_result++) {
                for (int index_of_digit_to_copy_from_non_normalised = index_of_first_one + 1;
                     index_of_digit_to_copy_from_non_normalised <
                     overall_form.length(); index_of_digit_to_copy_from_non_normalised++) {
                    res[index_in_result] = overall_form[index_of_digit_to_copy_from_non_normalised];
                }
            }
            overall_form = res;
        }
        return overall_form;
    }

    //  A helper method that transofrms input like '1101' to '1101000000', so that we can set these bits as mentissa bits
    string add_missing_zeroes(string str_to_add_zeroes_to, int length_of_output) {
        string res(length_of_output, '0');
        for(int i = 0; i < str_to_add_zeroes_to.length(); i++){
            res[i] = str_to_add_zeroes_to[i];
        }
        return res;
    }

    // Suppose normalised form is '1.1101'. We need to take only '1101' part, and this cycle does exactly this
    string parse_mentissa_part(string normalised_form) {
        string res = "";
        for(int i = 0; i < normalised_form.length() - 2; i++) {
            res += normalised_form[i+2];
        }
        return res;
    }

//    A method that takes in a string, like 7.025, splits it to integer part of 7 and float part of 0.25. It then converts 7 to base 2.
    void load_float(float float_to_load) {
        cout << "loading " << float_to_load << endl;
        int int_part = (int) float_to_load;
        cout << "integer part: " << int_part << endl;

        string int_part_in_binary = to_binary(int_part);
        cout << "integer part in binary: " << int_part_in_binary << endl;


        float float_part = (float) (float_to_load - int_part);
        cout << "float part:" << float_part << endl;

        // The input to floating_part_to_binary must be < 1, therefore if the float_to_load is 7.25, we need to pass 0.25 to floating_part_to_binary in order, to get the correct output
        string float_part_in_binary = floating_part_to_binary(float_to_load - (int) float_to_load);
        cout << "floating part in binary: " << float_part_in_binary << endl;

        non_normalised_form =  int_part_in_binary + "." + float_part_in_binary;
        cout << "Overall non-normalised digit: " << non_normalised_form << endl;

        normalised_form = calculate_normalised_form(non_normalised_form);
        shift  = calculate_shift(non_normalised_form);
        cout << "Overall normalised form: " << normalised_form << endl;
        cout << "Shift:" << shift << endl;

//        int exponent_bias = 2(num_of_characteristic_bits-1) -1;
        int exponent_bias = pow(2, num_of_characteristic_bits -1) - 1;
        cout << "exponent bias:" << exponent_bias << endl;

        int exponent = shift + exponent_bias;
        cout << "Exponent: " << exponent << endl;

        string exponent_in_binary = to_binary(exponent);
        set_characteristic_bits(exponent_in_binary);
        cout << "Exponent in binary: " << exponent_in_binary << endl;

        string mantissa_bits = parse_mentissa_part(normalised_form);
        mantissa_bits = add_missing_zeroes(mantissa_bits, num_of_mantissa_bits);
        cout << "A string that we need to add 0s to: " << mantissa_bits << endl;


        set_mantissa_bits(mantissa_bits);
    }
};

const int CHAR_BITS = 4;
const int MENTISSA_BITS = 10;

void print_headlines() {
    cout << 's' << SEPARATOR;

    for (int i = 0; i < CHAR_BITS; i++) {
        cout << 'h';
    }
    cout << SEPARATOR << 'n' << SEPARATOR;

    for (int i = 0; i < MENTISSA_BITS; i++) {
        cout << 'm';
    }
    cout << SEPARATOR << "Description" << endl;
}

int main() {
////   Zero
//    Digit_with_floating_point zero(CHAR_BITS, MENTISSA_BITS);
//    zero.set_mantissa_bits("0000000000");
//
//    zero.set_characteristic_bits("0000");
//    zero.set_sign_bit(0);
//    zero.set_implicit_bit(0);
//    zero.set_description("Zero");
//
////    +inf
//    Digit_with_floating_point positive_infinity(CHAR_BITS, MENTISSA_BITS);
//    positive_infinity.set_characteristic_bits("1111");
//    positive_infinity.set_mantissa_bits("0000000000");
//    positive_infinity.set_sign_bit(0);
//    positive_infinity.set_description("+ Inf");
//
////    -inf
//    Digit_with_floating_point negative_infinity(CHAR_BITS, MENTISSA_BITS);
//    negative_infinity.set_characteristic_bits("1111");
//    negative_infinity.set_mantissa_bits("0000000000");
//    negative_infinity.set_sign_bit(1);
//    negative_infinity.set_description("- Inf");
//
////
//    print_headlines();
//    zero.print_info();
//    positive_infinity.print_info();
//    negative_infinity.print_info();

    Digit_with_floating_point test(CHAR_BITS, MENTISSA_BITS);
    test.load_float(7.25);
    test.print_info();

//TODO: 1. review 'load_float' method.
// 2. Add the implicit bit setter
// 3. Add the factory of digits and special values, inherited from it.
    return 0;
}
