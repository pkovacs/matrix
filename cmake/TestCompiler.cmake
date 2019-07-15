include(CheckCSourceCompiles)

check_c_source_compiles("
   void free_buffer(char **p)
   {}

   int main(void)
   {
     __attribute__ ((cleanup(free_buffer))) char *p = 0;
     return 0;
   }
"  HAVE_ATTRIBUTE_CLEANUP
)
