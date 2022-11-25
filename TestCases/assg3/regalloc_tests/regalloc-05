/*
 * regalloc-05: register allocation for many variables in multiple basic blocks.
 *
 * In this program the variables are live for much longer and so each variable
 * interferes with many other variables.   The program tests how well the 
 * register allocator deals with spilling when coloring the interference graph.
 */

extern void println(int x);

void main(void) {
  int x01, x02, x03, x04, x05, x06, x07, x08, x09, x10;
  int y01, y02, y03, y04, y05, y06, y07, y08, y09, y10;
  int z01, z02, z03, z04, z05, z06, z07, z08, z09, z10;

  x01 = 1;
  y01 = x01 + 1;
  z01 = y01 * x01;
  x02 = x01 + 1;
  x03 = x02 + x01;
  x04 = x03 + x02;
  x05 = x04 + x03;

  if (x05 > 0) {
    x06 = x05 + x04;
    x07 = x06 + x05;
    x08 = x07 + x06;
    x09 = x08 + x07;
    x10 = x09 + x08;
    y01 = x10 + x09;
  }
  else {
    x06 = x05 - x04;
    x07 = x06 - x05;
    x08 = x07 - x06;
    x09 = x08 - x07;
    x10 = x09 - x08;
    y01 = x10 - x09;
  }
  
  y02 = y01 + 1;
  y03 = y02 + x01;
  y04 = y03 + x02;
  y05 = y04 + x03;
  y06 = y05 + x04;
  y07 = y06 + x05;
  y08 = y07 + x06;
  y09 = y08 + x07;
  y10 = y09 + x08;

  if (y10 != y01) {  
    z01 = y10 + z09;
    z02 = z01 + 1;
    z03 = x03 + z02 + y01;
    z04 = x04 + z03 + y02;
    z05 = x05 + z04 + y03;
  }
  else {
    z01 = y10 - z09;
    z02 = z01 - 1;
    z03 = x03 - z02 + y01;
    z04 = x04 - z03 + y02;
    z05 = x05 - z04 + y03;
  }

  z06 = x06 + z05 + y04;
  z07 = x07 + z06 + y05;
  z08 = x08 + z07 + y06;
  z09 = x09 + z08 + y07;
  z10 = x10 + z09 + y08;
  
  println(z10);
}
