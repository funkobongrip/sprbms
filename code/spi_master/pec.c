/*
 * Copyright (c) 2012 David Brandt
 *  All rights reserved.
 *
 * Licensed under: CC BY-NC-SA 3.0 (full license in file LICENSE)
 * 
 * You are free:
 * - to Share - to copy, distribute and transmit the work
 * - to Remix - to adapt the work
 * 
 * Under the following conditions:
 * - Attribution - You must attribute the work in the manner specified by
 *   the author or licensor (but not in any way that suggests that
 *   they endorse you or your use of the work).
 * 
 * - Noncommercial - You may not use this work for commercial purposes.
 * 
 * - Share Alike - If you alter, transform, or build upon this work, you may
 *   distribute the resulting work only under the same or similar license
 *   to this one.
 */unsigned char pec = 0;
void update_pec(unsigned char data)
{
  unsigned char i;
  unsigned char in[3];
  unsigned char tmp_pec;
  
  tmp_pec = pec;
  
#define pec_din(i) ((data >> i) & 0x01)
#define pec_val(i) ((tmp_pec >> i) & 0x01)
  for(i=8; i>0 ;)
  {  
    in[0] = pec_din((--i)) ^ pec_val(7);
    in[1] = pec_val(0) ^ in[0];
    in[2] = pec_val(1) ^ in[0];
    
    pec = 0;
    pec |= pec_val(6) << 7;
    pec |= pec_val(5) << 6;
    pec |= pec_val(4) << 5;
    pec |= pec_val(3) << 4;
    pec |= pec_val(2) << 3;
    pec |= (in[2] & 0x01) << 2;
    pec |= (in[1] & 0x01) << 1;
    pec |= (in[0] & 0x01) << 0;
    
    tmp_pec = pec;
  }
  pec = tmp_pec;
}