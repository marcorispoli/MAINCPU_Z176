#define _BIOPSY_DRIVER_C
#include "dbt_m4.h" 


/*
   GET_STAT()
   - reset: clear resettable status flags
*/
bool BiopsyDriverGetStat(unsigned char* statL, unsigned char* statH, bool reset)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x0;
    if(reset) tx_buffer[2] = 0x1;
    else tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *statL = rx_buffer[1];
      *statH = rx_buffer[2];
      return TRUE;
  }

  return FALSE;
}

/*
   GET_REVISION()
*/
bool BiopsyDriverGetRevision(unsigned char* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x3;
    tx_buffer[2] = 0x0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *val = rx_buffer[1];
      return TRUE;
  }

  return FALSE;
}

/*
   GET_CHECKSUM()
*/
bool BiopsyDriverGetChecksum(unsigned char* chkl, unsigned char* chkh)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x4;
    tx_buffer[2] = 0x0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *chkl = rx_buffer[1];
      *chkh = rx_buffer[2];
      return TRUE;
  }

  return FALSE;
}


/*
   GET_JOYX()
*/
bool BiopsyDriverGetJoyX(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x1;
    tx_buffer[2] = 0x0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *val = rx_buffer[1] + 256 * rx_buffer[2];
      return TRUE;
  }

  return FALSE;
}

/*
   GET_JOYY()
*/
bool BiopsyDriverGetJoyY(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x2;
    tx_buffer[2] = 0x0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *val = rx_buffer[1] + 256 * rx_buffer[2];
      return TRUE;
  }

  return FALSE;
}


/*
   GET_NEEDLE()
*/
bool BiopsyDriverGetNeedle(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x5;
    tx_buffer[2] = 0x0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

  if(rx_buffer[0]==tx_buffer[0])
  {
      *val = rx_buffer[1] + 256 * rx_buffer[2];
      return TRUE;
  }

  return FALSE;
}

bool BiopsyDriverSetStepVal(unsigned char val, unsigned char* stepval)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 0x7;
    tx_buffer[2] = val;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]){
        if(stepval)  *stepval =  rx_buffer[1];
        return true;
    }
    return FALSE;
}


/*
  La funzione chiede info sulla X corrente del posizionamento
*/
bool BiopsyDriverGetX(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8E;
    tx_buffer[1] = 0x0;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}


/*
  La funzione chiede info sulla Y corrente del posizionamento
*/
bool BiopsyDriverGetY(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8F;
    tx_buffer[1] = 0x0;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}


/*
  La funzione chiede info sulla Z corrente del posizionamento
*/
bool BiopsyDriverGetZ(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x90;
    tx_buffer[1] = 0x0;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}


/*
  La funzione chiede info sulla TGX corrente
*/
bool BiopsyDriverGetTGX(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8E;
    tx_buffer[1] = 0x1;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}

/*
  La funzione chiede info sulla TGY corrente
*/
bool BiopsyDriverGetTGY(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8F;
    tx_buffer[1] = 0x1;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}



/*
  La funzione chiede info sulla TGZ corrente
*/
bool BiopsyDriverGetTGZ(unsigned short* val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x90;
    tx_buffer[1] = 0x1;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0])
    {
        *val = rx_buffer[1] + 256 * rx_buffer[2];
        return TRUE;
    }

    return FALSE;
}


/*
  La funzione imposta il target TGX
*/
bool BiopsyDriverSetTGX(unsigned short val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0xCE;
    tx_buffer[1] = (unsigned char) (val & 0xFF);
    tx_buffer[2] = (unsigned char) ((val>>8) & 0xFF);

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) return true;
    return FALSE;
}


/*
  La funzione imposta il target TGY
*/
bool BiopsyDriverSetTGY(unsigned short val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0xCF;
    tx_buffer[1] = (unsigned char) (val & 0xFF);
    tx_buffer[2] = (unsigned char) ((val>>8) & 0xFF);

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) return true;
    return FALSE;

}


/*
  La funzione imposta il target TGZ
*/
bool BiopsyDriverSetTGZ(unsigned short val)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0xD0;
    tx_buffer[1] = (unsigned char) (val & 0xFF);
    tx_buffer[2] = (unsigned char) ((val>>8) & 0xFF);

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) return true;
    return FALSE;
}

bool BiopsyDriverMoveXYZ(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];
printf("BIOPSY DRIVER XYZ\n");
    tx_buffer[0] = 0x10;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}


bool BiopsyDriverMoveHOME(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x10;
    tx_buffer[1] = 3;
    tx_buffer[2] = 0;
printf("BIOPSY DRIVER HOME\n");
#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveDecZ(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x10;
    tx_buffer[1] = 1;
    tx_buffer[2] = 0;
printf("BIOPSY DRIVER DEC Z\n");
#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveIncZ(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x10;
    tx_buffer[1] = 2;
    tx_buffer[2] = 0;
printf("BIOPSY DRIVER INC Z\n");
#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveDecX(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];
printf("BIOPSY DRIVER DEC X\n");
    tx_buffer[0] = 0x0E;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveIncX(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x0E;
    tx_buffer[1] = 1;
    tx_buffer[2] = 0;
printf("BIOPSY DRIVER INC X\n");
#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveDecY(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x0F;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;
    printf("BIOPSY DRIVER DEC Y\n");
#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}

bool BiopsyDriverMoveIncY(unsigned char* statusL, unsigned char* statusH)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];
    printf("BIOPSY DRIVER INC Y\n");

    tx_buffer[0] = 0x0F;
    tx_buffer[1] = 1;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) {
        *statusL = rx_buffer[1];
        *statusH = rx_buffer[2];
        return true;
    }
    return FALSE;
}



bool BiopsyDriverReset(void)
{
    printf("BIOPSY DRIVER RESET\n");
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x8D;
    tx_buffer[1] = 6;
    tx_buffer[2] = 0;

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]) return true;
    return FALSE;
}

bool BiopsyDriverSetZlim(unsigned short val, unsigned short* zlim)
{
    unsigned char rx_buffer[4];
    unsigned char tx_buffer[4];

    tx_buffer[0] = 0x50;
    tx_buffer[1] = (unsigned char) (val & 0xFF);
    tx_buffer[2] = (unsigned char) ((val>>8) & 0xFF);

#ifdef __BIOPSY_SIMULATOR
    sim_serialCommand(tx_buffer,rx_buffer);
#else
    Ser422SendRaw(tx_buffer[0], tx_buffer[1], tx_buffer[2], rx_buffer, 5);
#endif

    if(rx_buffer[0]==tx_buffer[0]){
        if(zlim)  *zlim =  rx_buffer[1] + 256 * rx_buffer[2];
        return true;
    }
    return FALSE;
}




/* EOF */
 

