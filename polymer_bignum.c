#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>  // for strlen

///////////////////INTEGER///////////////////////

typedef struct { // structure of the integer
  unsigned char *arr;     // to store digits (0-9)
  unsigned long long size;      // current size (number of digits)
  unsigned long long cap;       // current capacity
  bool negative = false;          // is it negative?
  bool initialized = true;       // state, or killswitch if false
} plm_int;

/////////////INTEGER: FN TEMPLATES//////////////

void plm_int_init(plm_int *var);
void plm_int_set_str(plm_int *var, const char *str);
void plm_int_print(plm_int *var);
void plm_int_set_cap(plm_int *var, unsigned long long new_cap);
void plm_int_shift_left(plm_int *var, unsigned long long positions);
int plm_int_cmp(const plm_int *a, const plm_int *b);

void plm_int_free(plm_int *var);

void plm_int_add(plm_int *res, plm_int *left, plm_int *right);
void plm_int_sub(plm_int *res, plm_int *left, plm_int *right);
void plm_int_mul(plm_int *res, plm_int *left, plm_int *right);
void plm_int_div(plm_int *res, plm_int *left, plm_int *right);

///////////////////INTEGER/////////////////////

// Initialize the bignum structure
void plm_int_init(plm_int *var) {
  var->size = 0;         // Set the initial size to 0
  var->cap = 10;         // Set the initial capacity to 10
  var->negative = false; // Set negative flag to false
  var->initialized = true; // Mark as initialized

  // Allocate memory for the array
  var->arr = (unsigned char *)malloc(var->cap * sizeof(unsigned char));
  if (var->arr == NULL) {
    // Handle memory allocation failure
    printf("plm_fatal_error: memory allocation failed!\n");
    exit(12); // Exit with code 12 for memory failure
  }
}



// Set up a number using a string
void plm_int_set_str(plm_int *var, const char *str) {
  if (!var->initialized) {
    printf("plm_error: integer not initialized.\n");
    exit(13); // Exit with error code 13 for uninitialized struct
  }

  long long len = strlen(str);
  var->size = len;

  // If the number is negative, set the negative flag and adjust string
  if (str[0] == '-') {
    var->negative = true;
    str++; // Skip the negative sign
    len--; // Adjust the length
  }

  // Ensure there's enough capacity for the string length
  if (len > var->cap) {
    var->cap = len;
    var->arr = realloc(var->arr, var->cap * sizeof(unsigned char));
    if (var->arr == NULL) {
      printf("plm_fatal_error: memory allocation failed during resize!\n");
      exit(12); // Exit on memory resize failure
    }
  }

  // Store each digit as an unsigned char in the array
  for (long long i = 0; i < len; i++) {
    var->arr[i] = str[i] - '0'; // Convert char to long long and store in array
  }
}



// Print the bignum
void plm_int_print(plm_int *var) {
  if (!var->initialized) {
    printf("plm_error: integer not initialized.\n");
    exit(13); // Exit if integer not initialized
  }

  if (var->negative) {
    printf("-");  // Print negative sign if necessary
  }

  for (long long i = 0; i < var->size; i++) {
    printf("%u", var->arr[i]);  // Print each digit
  }
  printf("\n");
}



//set capacity of plm_int
void plm_int_set_cap(plm_int *var, unsigned long long new_cap){
  if (!var->initialized){
    printf("plm_error: integer trying to re-set capacity, not initialized");
    exit(13);
  }

  if (new_cap == var->cap) return;

  if (new_cap < var->size){
    printf("plm_error: integer trying to re-set capacity, has size larger than wanted new capacity");
    exit(14);
  }

  var->cap = new_cap;
  var->arr = realloc(var->arr, var->cap * sizeof(unsigned char));

  if (var->arr == NULL){
    printf("plm_fatal_error: memory allocation failed during resize!");
    exit(12);
  }
}

///////////////////INTEGER: SHIFTING AND COMPARING///////////////////////

void plm_int_shift_left(plm_int *var, unsigned long long positions) {
  if (!var->initialized || positions == 0) return;
  plm_int_set_cap(var, var->size + positions);
  for (long long i = var->size - 1; i >= 0; i--) {
    var->arr[i + positions] = var->arr[i];
  }
  for (long long i = 0; i < positions; i++) {
    var->arr[i] = 0;
  }
  var->size += positions;
}

long long plm_int_cmp(const plm_int *a, const plm_int *b) {
  if (!a->initialized || !b->initialized) {
    printf("plm_error: integers not initialized.\n");
    exit(13);
  }
  if (a->size != b->size) return a->size > b->size ? 1 : -1;
  for (long long i = a->size - 1; i >= 0; i--) {
    if (a->arr[i] != b->arr[i]) return a->arr[i] > b->arr[i] ? 1 : -1;
  }
  return 0;
}


//////////////////////////INTEGER: OPERATORS/////////////////////////////
// Add two plm_ints, handling negative values
void plm_int_add(plm_int *res, plm_int *left, plm_int *right) {
  if (!res->initialized) {
    printf("plm_error: result integer not initialized\n");
    exit(13);
  } else if (!left->initialized || !right->initialized) {
    printf("plm_error: left/right integer not initialized\n");
    exit(13);
  }

  // Reset result size
  res->size = 0;

  // If both numbers are negative, convert to positive and add
  if (left->negative && right->negative) {
    left->negative = false;
    right->negative = false;
    plm_int_add(res, left, right);  // Now both are positive, just add
    res->negative = true;  // Result is negative
    return;
  }

  // If one number is negative, subtract the smaller from the larger
  if (left->negative && !right->negative) {
    left->negative = false;
    plm_int_sub(res, right, left); // Right - Left
    res->negative = right->negative;
    return;
  }
  if (!left->negative && right->negative) {
    right->negative = false;
    plm_int_sub(res, left, right);  // Left - Right
    res->negative = left->negative;
    return;
  }

  // Both numbers are positive, add normally
  long long max_size = (left->size > right->size) ? left->size : right->size;
  res->size = max_size;

  plm_int_set_cap(res, max_size + 1);
  unsigned long long carry = 0;

  for (long long i = 0; i < max_size; i++) {
    unsigned char left_digit = (i < left->size) ? left->arr[i] : 0;
    unsigned char right_digit = (i < right->size) ? right->arr[i] : 0;

    unsigned long long sum = left_digit + right_digit + carry;
    res->arr[i] = sum % 10;
    carry = sum / 10;
  }

  if (carry) {
    res->arr[max_size] = carry;
    res->size++;
  }

  // Handle the size reduction for carry at the front
  while (res->size > 1 && res->arr[res->size - 1] == 0) {
    res->size--;
  }
}



// Subtract two plm_ints, handling negative values
void plm_int_sub(plm_int *res, plm_int *left, plm_int *right) {
  if (!res->initialized) {
    printf("plm_error: result integer not initialized\n");
    exit(13);
  } else if (!left->initialized || !right->initialized) {
    printf("plm_error: left/right integer not initialized\n");
    exit(13);
  }

  // Reset result size
  res->size = 0;

  // If both numbers are negative, convert to positive and subtract
  if (left->negative && right->negative) {
    left->negative = false;
    right->negative = false;
    plm_int_sub(res, right, left); // Subtract as if positive
    res->negative = true;  // Result is negative because left was larger
    return;
  }

  // If one number is negative, convert subtraction into addition
  if (left->negative && !right->negative) {
    left->negative = false;
    plm_int_add(res, left, right);  // Convert to addition
    res->negative = true;
    return;
  }
  if (!left->negative && right->negative) {
    right->negative = false;
    plm_int_add(res, left, right);  // Convert to addition
    res->negative = false;
    return;
  }

  // Now both numbers are positive
  if (left->size < right->size) {
    plm_int_sub(res, right, left);  // Swap left and right
    res->negative = true;
    return;
  }

  // Normal subtraction logic
  unsigned long long borrow = 0;
  long long max_size = left->size;
  res->size = max_size;

  plm_int_set_cap(res, max_size + 1);

  for (long long i = 0; i < max_size; i++) {
    long long digit1 = (i < left->size) ? left->arr[i] : 0;
    long long digit2 = (i < right->size) ? right->arr[i] : 0;

    long long diff = digit1 - digit2 - borrow;
    if (diff < 0) {
      diff += 10;
      borrow = 1;
    } else {
      borrow = 0;
    }

    res->arr[i] = diff;
  }

  // Handle the size reduction for borrow at the front
  while (res->size > 1 && res->arr[res->size - 1] == 0) {
    res->size--;
  }
}



// Multiply two plm_ints
void plm_int_mul(plm_int *res, plm_int *left, plm_int *right) {
  if (!res->initialized) {
    printf("plm_error: result integer not initialized\n");
    exit(13);
  } else if (!left->initialized || !right->initialized) {
    printf("plm_error: left/right integer not initialized\n");
    exit(13);
  }

  // Initialize result
  res->size = left->size + right->size - 1;
  plm_int_set_cap(res, res->size+1);

  // Handle negative sign
  bool negative_result = (left->negative != right->negative);
  res->negative = negative_result;

  for (long long i = 0; i < left->size; i++) {
    for (long long j = 0; j < right->size; j++) {
      long long product = left->arr[i] * right->arr[j];
      long long pos = i + j;
      res->arr[pos] += product;
      
      // Handle carry
      if (res->arr[pos] >= 10) {
        res->arr[pos + 1] += res->arr[pos] / 10;
        res->arr[pos] %= 10;
      }
    }
  }

  // Handle leading zeros
  while (res->size > 1 && res->arr[0] == 0) {
    // shift array elements left by 1 position
    for (long long i = 0; i < res->size - 1; i++) {
      res->arr[i] = res->arr[i + 1];
    }
    res->size--;
  }
 
}



// Divide two plm_ints
void plm_int_div(plm_int *res, plm_int *left, plm_int *right) {
  if (!res->initialized) {
    printf("plm_error: result integer not initialized\n");
    exit(13);
  } else if (!left->initialized || !right->initialized) {
    printf("plm_error: left/right integer not initialized\n");
    exit(13);
  }

  // Handle division by zero
  if (right->size == 1 && right->arr[0] == 0) {
    printf("plm_fatal_error: division by zero!\n");
    exit(15);
  }

  // Initialize result size
  res->size = left->size;
  plm_int_set_cap(res, res->size+1);

  bool negative_result = (left->negative != right->negative);
  res->negative = negative_result;

  // Initialize the remainder as an empty plm_int
  plm_int remainder;
  plm_int_init(&remainder);
  
  // Process each digit of the left number
  for (long long i = left->size - 1; i >= 0; i--) {
    // Update the remainder (shift left and add current digit of left)
    plm_int_shift_left(&remainder, 1);  // Shift the remainder
    remainder.arr[0] = left->arr[i];

    // Find how many times the divisor fits into the remainder (quotient digit)
    long long quotient_digit = 0;
    while (plm_int_cmp(&remainder, right) >= 0) {
      // Subtract the divisor from remainder (remainder -= right)
      plm_int_sub(&remainder, &remainder, right);
      quotient_digit++;
    }

    // Store the quotient digit
    res->arr[i] = quotient_digit;
  }

  // Handle leading zeros in the result
  while (res->size > 1 && res->arr[0] == 0) {
    // shift array elements left by 1 position
    for (long long i = 0; i < res->size - 1; i++) {
      res->arr[i] = res->arr[i + 1];
    }
    res->size--;
  }


  // Free the remainder memory
  plm_int_free(&remainder);
  }


///////////////////////////INTEGER:FREE//////////////////////////////////

void plm_int_free(plm_int *var){
  if (var->arr != NULL && var->initialized){
    free(var->arr);
    var->arr = NULL;
    var->initialized = false;
  }
}

///////////////////////////END OF INTEGER////////////////////////////////


////////////////////////////////////////////////////////////////////////


///////////////////////////////FLOAT////////////////////////////////////
typedef struct {
  unsigned char *arr;
  unsigned long long size;
  unsigned long long cap;
  unsigned long long decimal_point_pos;
  bool negative;
  bool initialized = false;
} plm_float;

/////////////////////FLOAT: FUNCTION PROTOTYPES/////////////////////////

void plm_float_init(plm_float *var);
void plm_float_set_str(plm_float *var, const char *str);
void plm_float_print(plm_float *var);
int plm_float_cmp(plm_float *var);

void plm_float_free(plm_float *var);

void plm_float_add(plm_float *res, plm_float *left, plm_float *right);
void plm_float_sub(plm_float *res, plm_float *left, plm_float *right);
void plm_float_mul(plm_float *res, plm_float *left, plm_float *right);
void plm_float_div(plm_float *res, plm_float *left, plm_float *right);

///////////////////////////FLOAT FUNCTIONS//////////////////////////////

typedef struct {
  unsigned char *arr;        // to store digits
  unsigned long long size;   // size of the significant part
  unsigned long long cap;    // capacity of the array
  long long exponent;        // position of the decimal point
  bool negative;             // is it negative?
  bool initialized;          // is the struct initialized
} plm_float;


void plm_float_set_str(plm_float *var, const char *str) {
  if (!var->initialized) {
    printf("plm_error: float not initialized.\n");
    exit(13);
  }
  long long len = strlen(str);
  long long dot_pos = -1;
  for (long long i = 0; i < len; i++) {
    if (str[i] == '.') {
      dot_pos = i;
      break;
    }
  }
  var->size = (dot_pos == -1) ? len : len - 1;
  if (dot_pos != -1) {
    var->exponent = dot_pos - var->size;
  }
  if (str[0] == '-') {
    var->negative = true;
    str++;
    len--;
  }
  if (var->size > var->cap) {
    var->cap = var->size;
    var->arr = realloc(var->arr, var->cap * sizeof(unsigned char));
    if (var->arr == NULL) {
      printf("plm_fatal_error: memory allocation failed during resize!\n");
      exit(12);
    }
  }
  long long index = 0;
  for (long long i = 0; i < len; i++) {
    if (str[i] != '.') {
      var->arr[index++] = str[i] - '0';
    }
  }
}


void plm_float_print(plm_float *var) {
  if (!var->initialized) {
    printf("plm_error: float not initialized.\n");
    exit(13);
  }
  if (var->negative) {
    printf("-");
  }
  for (long long i = 0; i < var->size; i++) {
    if (i == var->size + var->exponent) {
      printf(".");
    }
    printf("%u", var->arr[i]);
  }
  printf("\n");
}


int plm_float_cmp(plm_float *left, plm_float *right) {
  if (!left->initialized || !right->initialized) printf("plm_error: left or right float not initialized.\n"); exit(13);
  if (left->size != right->size) return left->size > right->size ? 1 : -1;

  for ()
}


void plm_float_free(plm_float *var) {
  if (var->arr) {
    free(var->arr);
  }
  var->initialized = false;
}

//////////////////////FLOAT: OPERATORS/////////////////////////////////

void plm_float_add(plm_float *res, plm_float *left, plm_float *right) {
    // Align exponents first
    long long exp_diff = left->exponent - right->exponent;
    long long max_size = left->size > right->size ? left->size : right->size;
    long long min_size = left->size < right->size ? left->size : right->size;

    // Adjust exponents by shifting digits
    if (exp_diff > 0) {
        // Shift right number's digits
        // (add zeros at the start of arr or other logic)
    } else if (exp_diff < 0) {
        // Shift left number's digits
    }

    // Perform digit-by-digit addition
    long long carry = 0;
    for (long long i = 0; i < max_size; i++) {
        long long left_digit = (i < left->size) ? left->arr[i] : 0;
        long long right_digit = (i < right->size) ? right->arr[i] : 0;
        long long sum = left_digit + right_digit + carry;

        res->arr[i] = sum % 10;
        carry = sum / 10;
    }

    // Handle carry if any
    if (carry) {
        res->arr[max_size] = carry;
        max_size++;
    }

    // Set the result's size and exponent
    res->size = max_size;
    res->exponent = left->exponent;  // Just an example, update accordingly
    res->negative = left->negative;  // You can handle the sign separately if needed
}


void plm_float_sub(plm_float *res, plm_float *left, plm_float *right) {
    // Similar to addition but subtract digit-by-digit
    // Align exponents first
    long long exp_diff = left->exponent - right->exponent;

    // Perform digit-by-digit subtraction
    long long borrow = 0;
    for (long long i = 0; i < left->size; i++) {
        long long left_digit = left->arr[i];
        long long right_digit = (i < right->size) ? right->arr[i] : 0;
        long long diff = left_digit - right_digit - borrow;

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }

        res->arr[i] = diff;
    }

    // Set the result's size and exponent
    res->size = left->size;
    res->exponent = left->exponent;
    res->negative = left->negative;
}


void plm_float_mul(plm_float *res, plm_float *left, plm_float *right) {
    // Prepare result size and exponents
    long long result_size = left->size + right->size;
    long long result_exp = left->exponent + right->exponent; // Add exponents

    // Perform digit-by-digit multiplication
    long long carry = 0;
    for (long long i = 0; i < left->size; i++) {
        for (long long j = 0; j < right->size; j++) {
            long long prod = left->arr[i] * right->arr[j] + carry;
            long long result_index = i + j;

            res->arr[result_index] += prod % 10; // Store the digit
            carry = prod / 10; // Carry for next place value
        }
    }

    // If there's carry left, handle it
    if (carry) {
        res->arr[result_size] = carry;
        result_size++;
    }

    // Normalize the result: shift digits if necessary
    int shift = 0;
    while (res->arr[shift] == 0 && shift < result_size) shift++;
    for (long long i = shift; i < result_size; i++) {
        res->arr[i - shift] = res->arr[i];
    }
    result_size -= shift;  // Adjust size after shift

    // Set the result's size, exponent, and sign
    res->size = result_size;
    res->exponent = result_exp;
    res->negative = left->negative != right->negative;
}


void plm_float_div(plm_float *res, plm_float *left, plm_float *right) {
    // Prepare result size and exponents
    long long result_size = left->size;
    long long result_exp = left->exponent - right->exponent; // Subtract exponents

    // Perform digit-by-digit division
    long long remainder = 0;
    for (long long i = 0; i < left->size; i++) {
        remainder = remainder * 10 + left->arr[i];
        res->arr[i] = remainder / right->arr[0]; // Simple division by the first digit of the divisor
        remainder %= right->arr[0]; // Update remainder
    }

    // Normalize the result: adjust for decimal point placement if needed
    int shift = 0;
    while (res->arr[shift] == 0 && shift < result_size) shift++;
    for (long long i = shift; i < result_size; i++) {
        res->arr[i - shift] = res->arr[i];
    }
    result_size -= shift;  // Adjust size after shift

    // Set the result's size, exponent, and sign
    res->size = result_size;
    res->exponent = result_exp;
    res->negative = left->negative != right->negative;
}

////////////////////END OF FLOAT////////////////////////////////
