#include "./polymer_bignum.c"

int main(int argc, char *argv[]){
  plm_int a,b,res;
  
  plm_int_init(&a);
  plm_int_init(&b);
  plm_int_init(&res);

  plm_int_set_str(&a, "100");
  plm_int_set_str(&b, "10");

  plm_int_mul(&res, &a, &b);
  
  printf("Result: ");
  plm_int_print(&res);

  plm_int_free(&a);
  plm_int_free(&b);
  plm_int_free(&res);

  return 0;
}
