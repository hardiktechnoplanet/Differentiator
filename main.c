#include <msp430.h>

unsigned int DMA_DST;						// ADC conversion result is stored in this variable
float results[100];
float results1[100];

//static unsigned int i=0;
int main(void)
{
  WDTCTL = WDTPW+WDTHOLD;                   // Hold WDT

  P1OUT &= ~BIT0;                           // P1.0 clear
  P1DIR |= BIT0;                            // P1.0 output
  P4OUT &= ~BIT7;                           // P1.0 clear
  P4DIR |= BIT7;                            // P1.0 output
  //P5SEL |= BIT7;                            // P5.7/TB1 option select
  //P5DIR |= BIT7;                            // Output direction
  P6SEL |= BIT0;                            // Enable A/D channel A0
  P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 SPI Option
  /////////////////////////////////////////////////////////////////////Setup Timer B0//////////////////////////////////////////////////////////////////////

  TBCCR0 = 0xffff;
  TBCCR1 = 0x0001;
  TBCCTL0 = CCIE;                           // CCR0 interrupt enabled
  TBCCTL1 = OUTMOD_4;                       // CCR1 set/reset mode
  TBCTL = TBSSEL_1+MC_2+TBCLR+ID_3;              // SMCLK, Up-Mode, clear TBR, divider for input clock

    /*TBCCR0 = 0xffff;

    TBCCTL0 |= CCIE +CM_1+CAP;                           // CCR0 interrupt enabled
    TBCCTL1 = OUTMOD_3;                       // CCR1 set/reset mode
    TBCTL |= TBSSEL_1+MC_2+TBIE;*/


  /////////////////////////////////////////////////////SET UP ADC_12////////////////////////////////////////////////////////////////////////////
  REFCTL0 &= ~REFMSTR;
    ADC12CTL0 = ADC12ON+ADC12SHT03+ADC12MSC+ADC12REFON+ADC12REF2_5V;         // Turn on ADC12, Sampling time (it controls the interval of sampling timer), MSC (multiple sample & convert bit)
    ADC12CTL1 = ADC12SHS_3+ADC12CONSEQ_2;
    ADC12CTL1 |= ADC12SHP+ADC12DIV_4;                     // Use sampling timer
    ADC12CTL2 |= ADC12PDIV ;
    ADC12CTL2 &= ~ADC12RES_2 ;
   // ADC12IE = 0x01;                           // Enable interrupt
    ADC12MCTL0 = ADC12SREF_1+ADC12INCH_0;
    ADC12CTL0 |= ADC12ENC;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////Setup DMA0///////////////////////////////////////////////////////////////
  DMACTL0 = DMA0TSEL_24;                    // ADC12IFGx triggered
  DMACTL4 = DMARMWDIS;                      // Read-modify-write disable
  DMA0CTL &= ~DMAIFG;
  DMA0CTL = DMADT_4+DMAEN+DMADSTINCR_3+DMAIE; // Rpt single tranfer, inc dst, Int
  DMA0SZ = 1;                               // DMA0 size = 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////SPI/////////////////////////////////////////////////////////
  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL0 = UCMST+UCSYNC+UCCKPL+UCMSB;     // 3-pin, 8-bit SPI master
                                              // Clock polarity high, MSB
    UCA0CTL1 = UCSSEL_2;                      // SMCLK
    UCA0BR0 = 0x02;                           // /2
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = 0x00;                          // No modulation
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



   while (1)
    {
      ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion
  __data16_write_addr((unsigned short) &DMA0SA,(unsigned long) &ADC12MEM0);
                                            // Source block address
  __data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &DMA_DST);
                                            // Destination single address
  //v=DMA_DST*2.5/4095;
  //results[i]=v;

  __bis_SR_register(GIE);       // LPM0 w/ interrupts
  __no_operation();                         // used for debugging
}}

//------------------------------------------------------------------------------
// DMA Interrupt Service Routine
//------------------------------------------------------------------------------
#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
	static float v=0;
	static unsigned int i=0;
	static unsigned int k=0;

  switch(__even_in_range(DMAIV,16))
  {
    case 0: break;
    case 2:                                 // DMA0IFG = DMA Channel 0
    	 //if (ADC12MEM0 >= 0x7ff)                 // ADC12MEM = A0 > 0.5AVcc?
      P1OUT ^= BIT0;                        // Toggle P1.0 - PLACE BREAKPOINT HERE AND CHECK DMA_DST VARIABLE

      //results[i]=DMA_DST;
      v=DMA_DST*(2.5/255);
     results[i]=v;
     i++;
     results1[k]=(results[i+1]-results[i])/0.001;
    // UCA0TXBUF=results[i];


    	    k++;
    	   /*if(i==200)
    	    {
    	    i=0;
    	    }*/
      break;
    case 4: break;                          // DMA1IFG = DMA Channel 1
    case 6: break;                          // DMA2IFG = DMA Channel 2
    case 8: break;                          // DMA3IFG = DMA Channel 3
    case 10: break;                         // DMA4IFG = DMA Channel 4
    case 12: break;                         // DMA5IFG = DMA Channel 5
    case 14: break;                         // DMA6IFG = DMA Channel 6
    case 16: break;                         // DMA7IFG = DMA Channel 7
    default: break;
  }
}
// Timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void)
{
	//static unsigned int i=0;4	ADC12CTL0 &=~ ADC12ENC;
		   ADC12CTL0 &=~ ADC12SC;
		   P4OUT ^= BIT7;
}


