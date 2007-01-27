#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

/************************************************************* 
*                                                            * 
*      New ISO to EBCDIC table : name ASCEBC                 *
*                                                            * 
**************************************************************/
unsigned char ASCEBC[256]= {
  /*
                                  ISO name (with common usage names) 
  */
    
    /* ITOE_00 */ 0X00,  /* NUL   null */
    /* ITOE_01 */ 0X01,  /* SOH   start of heading           (Ctrl-A) */
    /* ITOE_02 */ 0X02,  /* STX   start of text              (Ctrl-B) */
    /* ITOE_03 */ 0X03,  /* ETX   end of text                (Ctrl-C) */
    /* ITOE_04 */ 0X37,  /* EOT   end of transmission        (Ctrl-D) */
    /* ITOE_05 */ 0X2D,  /* ENQ   enquiry                    (Ctrl-E) */
    /* ITOE_06 */ 0X2E,  /* ACK   acknowledge                (Ctrl-F) */
    /* ITOE_07 */ 0X2F,  /* BEL   bell                       (Ctrl-G) */
    /* ITOE_08 */ 0X16,  /* BS    backspace                  (Ctrl-H) */
    /* ITOE_09 */ 0X05,  /* HT    horizontal tabulation      (Ctrl-I) */
    /* ITOE_0A */ 0X25,  /* LF    line feed                  (Ctrl-J) */
    /* ITOE_0B */ 0X0B,  /* VT    vertical tabulation        (Ctrl-K) */
    /* ITOE_0C */ 0X0C,  /* FF    form feed                  (Ctrl-L) */
    /* ITOE_0D */ 0X0D,  /* CR    carriage return            (Ctrl-M) */
    /* ITOE_0E */ 0X0E,  /* SO    shift-out                  (Ctrl-N) */
    /* ITOE_0F */ 0X0F,  /* SI    shift-in                   (Ctrl-O) */
/**/
    /* ITOE_10 */ 0X10,  /* DLE   data link escape           (Ctrl-P) */
    /* ITOE_11 */ 0X11,  /* DC1   device control 1    (X-Off, Ctrl-Q) */
    /* ITOE_12 */ 0X12,  /* DC2   device control 2           (Ctrl-R) */
    /* ITOE_13 */ 0X13,  /* DC3   device control 3     (X-On, Ctrl-S) */
    /* ITOE_14 */ 0X3C,  /* DC4   device control 4           (Ctrl-T) */
    /* ITOE_15 */ 0X3D,  /* NAK   negative acknowledge       (Ctrl-U) */
    /* ITOE_16 */ 0X32,  /* SYN   synchronous idle           (Ctrl-V) */
    /* ITOE_17 */ 0X26,  /* ETB   end of transmission block  (Ctrl-W) */
    /* ITOE_18 */ 0X18,  /* CAN   cancel                     (Ctrl-X) */
    /* ITOE_19 */ 0X19,  /* EM    end of medium              (Ctrl-Y) */
    /* ITOE_1A */ 0X3F,  /* SUB   substitute character       (Ctrl-Z) */
    /* ITOE_1B */ 0X27,  /* ESC   escape                     (Escape) */
    /* ITOE_1C */ 0X1C,  /* FS    file separator */
    /* ITOE_1D */ 0X1D,  /* GS    group separator */
    /* ITOE_1E */ 0X1E,  /* RS    record separator */
    /* ITOE_1F */ 0X1F,  /* US    unit separator */
/**/
    /* ITOE_20 */ 0X40,  /* space (blank) */
    /* ITOE_21 */ 0X5A,  /* exclamation mark */
    /* ITOE_22 */ 0X7F,  /* quotation mark (double quote) */
    /* ITOE_23 */ 0X7B,  /* number sign (hash mark, sharp sign) */
    /* ITOE_24 */ 0X5B,  /* dollar sign */
    /* ITOE_25 */ 0X6C,  /* percent sign */
    /* ITOE_26 */ 0X50,  /* ampersand (and sign) */
    /* ITOE_27 */ 0X7D,  /* apostrophe (single quote) */
    /* ITOE_28 */ 0X4D,  /* left parenthesis */
    /* ITOE_29 */ 0X5D,  /* right parenthesis */
    /* ITOE_2A */ 0X5C,  /* asterisk (star) */
    /* ITOE_2B */ 0X4E,  /* plus sign */
    /* ITOE_2C */ 0X6B,  /* comma */
    /* ITOE_2D */ 0X60,  /* minus sign or hyphen */
    /* ITOE_2E */ 0X4B,  /* period, full stop */
    /* ITOE_2F */ 0X61,  /* solidus (slash) */
/**/
    /* ITOE_30 */ 0XF0,  /* digit zero */
    /* ITOE_31 */ 0XF1,  /* digit one */
    /* ITOE_32 */ 0XF2,  /* digit two */
    /* ITOE_33 */ 0XF3,  /* digit three */
    /* ITOE_34 */ 0XF4,  /* digit four */
    /* ITOE_35 */ 0XF5,  /* digit five */
    /* ITOE_36 */ 0XF6,  /* digit six */
    /* ITOE_37 */ 0XF7,  /* digit seven */
    /* ITOE_38 */ 0XF8,  /* digit eight */
    /* ITOE_39 */ 0XF9,  /* digit nine */
    /* ITOE_3A */ 0X7A,  /* colon */
    /* ITOE_3B */ 0X5E,  /* semicolon */
    /* ITOE_3C */ 0X4C,  /* less-than sign */
    /* ITOE_3D */ 0X7E,  /* equals sign */
    /* ITOE_3E */ 0X6E,  /* greater-than sign */
    /* ITOE_3F */ 0X6F,  /* question mark */
/**/
    /* ITOE_40 */ 0X7C,  /* commercial at */
    /* ITOE_41 */ 0XC1,  /* capital A */
    /* ITOE_42 */ 0XC2,  /* capital B */
    /* ITOE_43 */ 0XC3,  /* capital C */
    /* ITOE_44 */ 0XC4,  /* capital D */
    /* ITOE_45 */ 0XC5,  /* capital E */
    /* ITOE_46 */ 0XC6,  /* capital F */
    /* ITOE_47 */ 0XC7,  /* capital G */
    /* ITOE_48 */ 0XC8,  /* capital H */
    /* ITOE_49 */ 0XC9,  /* capital I */
    /* ITOE_4A */ 0XD1,  /* capital J */
    /* ITOE_4B */ 0XD2,  /* capital K */
    /* ITOE_4C */ 0XD3,  /* capital L */
    /* ITOE_4D */ 0XD4,  /* capital M */
    /* ITOE_4E */ 0XD5,  /* capital N */
    /* ITOE_4F */ 0XD6,  /* capital O */
/**/
    /* ITOE_50 */ 0XD7,  /* capital P */
    /* ITOE_51 */ 0XD8,  /* capital Q */
    /* ITOE_52 */ 0XD9,  /* capital R */
    /* ITOE_53 */ 0XE2,  /* capital S */
    /* ITOE_54 */ 0XE3,  /* capital T */
    /* ITOE_55 */ 0XE4,  /* capital U */
    /* ITOE_56 */ 0XE5,  /* capital V */
    /* ITOE_57 */ 0XE6,  /* capital W */
    /* ITOE_58 */ 0XE7,  /* capital X */
    /* ITOE_59 */ 0XE8,  /* capital Y */
    /* ITOE_5A */ 0XE9,  /* capital Z */
    /* ITOE_5B */ 0XBA,  /* left square bracket */
    /* ITOE_5C */ 0XE0,  /* reverse solidus (backslash) */
    /* ITOE_5D */ 0XBB,  /* right square bracket */
    /* ITOE_5E */ 0XB0,  /* circumflex accent */
    /* ITOE_5F */ 0X6D,  /* low line (underscore) */
/**/
    /* ITOE_60 */ 0X79,  /* grave accent */
    /* ITOE_61 */ 0X81,  /* small a */
    /* ITOE_62 */ 0X82,  /* small b */
    /* ITOE_63 */ 0X83,  /* small c */
    /* ITOE_64 */ 0X84,  /* small d */
    /* ITOE_65 */ 0X85,  /* small e */
    /* ITOE_66 */ 0X86,  /* small f */
    /* ITOE_67 */ 0X87,  /* small g */
    /* ITOE_68 */ 0X88,  /* small h */
    /* ITOE_69 */ 0X89,  /* small i */
    /* ITOE_6A */ 0X91,  /* small j */
    /* ITOE_6B */ 0X92,  /* small k */
    /* ITOE_6C */ 0X93,  /* small l */
    /* ITOE_6D */ 0X94,  /* small m */
    /* ITOE_6E */ 0X95,  /* small n */
    /* ITOE_6F */ 0X96,  /* small o */
/**/
    /* ITOE_70 */ 0X97,  /* small p */
    /* ITOE_71 */ 0X98,  /* small q */
    /* ITOE_72 */ 0X99,  /* small r */
    /* ITOE_73 */ 0XA2,  /* small s */
    /* ITOE_74 */ 0XA3,  /* small t */
    /* ITOE_75 */ 0XA4,  /* small u */
    /* ITOE_76 */ 0XA5,  /* small v */
    /* ITOE_77 */ 0XA6,  /* small w */
    /* ITOE_78 */ 0XA7,  /* small x */
    /* ITOE_79 */ 0XA8,  /* small y */
    /* ITOE_7A */ 0XA9,  /* small z */
    /* ITOE_7B */ 0XC0,  /* left curly bracket (left brace) */
    /* ITOE_7C */ 0X4F,  /* vertical line (bar, "or" sign) */
    /* ITOE_7D */ 0XD0,  /* right curly bracket (right brace) */
    /* ITOE_7E */ 0XA1,  /* tilde (wavy line) */
    /* ITOE_7F */ 0X07,  /* DEL   delete (rubout, DEL control char) */
/**/
    /* ITOE_80 */ 0X20,  /* ... */
    /* ITOE_81 */ 0X21,  /* ... */
    /* ITOE_82 */ 0X22,  /* ... */
    /* ITOE_83 */ 0X23,  /* ... */
    /* ITOE_84 */ 0X24,  /* ... */
    /* ITOE_85 */ 0X15,  /* ... */
    /* ITOE_86 */ 0X06,  /* ... */
    /* ITOE_87 */ 0X17,  /* ... */
    /* ITOE_88 */ 0X28,  /* ... */
    /* ITOE_89 */ 0X29,  /* ... */
    /* ITOE_8A */ 0X2A,  /* ... */
    /* ITOE_8B */ 0X2B,  /* ... */
    /* ITOE_8C */ 0X2C,  /* ... */
    /* ITOE_8D */ 0X09,  /* ... */
    /* ITOE_8E */ 0X0A,  /* ... */
    /* ITOE_8F */ 0X1B,  /* ... */
/**/
    /* ITOE_90 */ 0X30,  /* ... */
    /* ITOE_91 */ 0X31,  /* ... */
    /* ITOE_92 */ 0X1A,  /* ... */
    /* ITOE_93 */ 0X33,  /* ... */
    /* ITOE_94 */ 0X34,  /* ... */
    /* ITOE_95 */ 0X35,  /* ... */
    /* ITOE_96 */ 0X36,  /* ... */
    /* ITOE_97 */ 0X08,  /* ... */
    /* ITOE_98 */ 0X38,  /* ... */
    /* ITOE_99 */ 0X39,  /* ... */
    /* ITOE_9A */ 0X3A,  /* ... */
    /* ITOE_9B */ 0X3B,  /* ... */
    /* ITOE_9C */ 0X04,  /* ... */
    /* ITOE_9D */ 0X14,  /* ... */
    /* ITOE_9E */ 0X3E,  /* ... */
    /* ITOE_9F */ 0XFF,  /* ... */
/**/
    /* ITOE_A0 */ 0X41,  /* no-break space */
    /* ITOE_A1 */ 0XAA,  /* inverted exclamation mark */
    /* ITOE_A2 */ 0X4A,  /* cent sign */
    /* ITOE_A3 */ 0XB1,  /* pound sign (Sterling currency) */
    /* ITOE_A4 */ 0X9F,  /* currency sign (lozenge) */
    /* ITOE_A5 */ 0XB2,  /* yen sign (Nipponese currency) */
    /* ITOE_A6 */ 0X6A,  /* broken bar */
    /* ITOE_A7 */ 0XB5,  /* section sign (S-half-above-S sign) */
    /* ITOE_A8 */ 0XBD,  /* diaeresis or umlaut */
    /* ITOE_A9 */ 0XB4,  /* copyright sign (circled capital C) */
    /* ITOE_AA */ 0X9A,  /* ordinal indicator feminine */
    /* ITOE_AB */ 0X8A,  /* angle quotation mark left (<< mark) */
    /* ITOE_AC */ 0X5F,  /* not sign */
    /* ITOE_AD */ 0XCA,  /* soft hyphen */
    /* ITOE_AE */ 0XAF,  /* registered sign (circled capital R) */
    /* ITOE_AF */ 0XBC,  /* macron */
/**/
    /* ITOE_B0 */ 0X90,  /* degree sign */
    /* ITOE_B1 */ 0X8F,  /* plus or minus sign */
    /* ITOE_B2 */ 0XEA,  /* superscript two (squared) */
    /* ITOE_B3 */ 0XFA,  /* superscript three (cubed) */
    /* ITOE_B4 */ 0XBE,  /* acute accent */
    /* ITOE_B5 */ 0XA0,  /* micro sign (small mu) */
    /* ITOE_B6 */ 0XB6,  /* pilcrow (paragraph, double-barred P) */
    /* ITOE_B7 */ 0XB3,  /* middle dot (scalar product) */
    /* ITOE_B8 */ 0X9D,  /* cedilla */
    /* ITOE_B9 */ 0XDA,  /* superscript one */
    /* ITOE_BA */ 0X9B,  /* ordinal indicator, masculine */
    /* ITOE_BB */ 0X8B,  /* angle quotation mark right (>> mark) */
    /* ITOE_BC */ 0XB7,  /* fraction one-quarter (1/4) */
    /* ITOE_BD */ 0XB8,  /* fraction one-half (1/2) */
    /* ITOE_BE */ 0XB9,  /* fraction three-quarters (3/4) */
    /* ITOE_BF */ 0XAB,  /* inverted question mark */
/**/
    /* ITOE_C0 */ 0X64,  /* capital A with grave accent */
    /* ITOE_C1 */ 0X65,  /* capital A with acute accent */
    /* ITOE_C2 */ 0X62,  /* capital A with circumflex accent */
    /* ITOE_C3 */ 0X66,  /* capital A with tilde */
    /* ITOE_C4 */ 0X63,  /* capital A with diaeresis */
    /* ITOE_C5 */ 0X67,  /* capital A with ring */
    /* ITOE_C6 */ 0X9E,  /* capital AE dipthong */
    /* ITOE_C7 */ 0X68,  /* capital C with cedilla */
    /* ITOE_C8 */ 0X74,  /* capital E with grave accent */
    /* ITOE_C9 */ 0X71,  /* capital E with acute accent */
    /* ITOE_CA */ 0X72,  /* capital E with circumflex accent */
    /* ITOE_CB */ 0X73,  /* capital E with diaeresis */
    /* ITOE_CC */ 0X78,  /* capital I with grave accent */
    /* ITOE_CD */ 0X75,  /* capital I with acute accent */
    /* ITOE_CE */ 0X76,  /* capital I with circumflex accent */
    /* ITOE_CF */ 0X77,  /* capital I with diaeresis */
/**/
    /* ITOE_D0 */ 0XAC,  /* capital D with stroke, Icelandic eth */
    /* ITOE_D1 */ 0X69,  /* capital N with tilde */
    /* ITOE_D2 */ 0XED,  /* capital O with grave accent */
    /* ITOE_D3 */ 0XEE,  /* capital O with acute accent */
    /* ITOE_D4 */ 0XEB,  /* capital O with circumflex accent */
    /* ITOE_D5 */ 0XEF,  /* capital O with tilde */
    /* ITOE_D6 */ 0XEC,  /* capital O with diaeresis */
    /* ITOE_D7 */ 0XBF,  /* multiply sign (vector product) */
    /* ITOE_D8 */ 0X80,  /* capital O with slash */
    /* ITOE_D9 */ 0XFD,  /* capital U with grave accent */
    /* ITOE_DA */ 0XFE,  /* capital U with acute accent */
    /* ITOE_DB */ 0XFB,  /* capital U with circumflex accent */
    /* ITOE_DC */ 0XFC,  /* capital U with diaeresis */
    /* ITOE_DD */ 0XAD,  /* capital Y with acute accent */
    /* ITOE_DE */ 0XAE,  /* capital thorn, Icelandic */
    /* ITOE_DF */ 0X59,  /* small sharp s, German */
/**/
    /* ITOE_E0 */ 0X44,  /* small a with grave accent */
    /* ITOE_E1 */ 0X45,  /* small a with acute accent */
    /* ITOE_E2 */ 0X42,  /* small a with circumflex accent */
    /* ITOE_E3 */ 0X46,  /* small a with tilde */
    /* ITOE_E4 */ 0X43,  /* small a with diaeresis */
    /* ITOE_E5 */ 0X47,  /* small a with ring above */
    /* ITOE_E6 */ 0X9C,  /* small ae dipthong */
    /* ITOE_E7 */ 0X48,  /* small c with cedilla */
    /* ITOE_E8 */ 0X54,  /* small e with grave accent */
    /* ITOE_E9 */ 0X51,  /* small e with acute accent */
    /* ITOE_EA */ 0X52,  /* small e with circumflex accent */
    /* ITOE_EB */ 0X53,  /* small e with diaeresis */
    /* ITOE_EC */ 0X58,  /* small i with grave accent */
    /* ITOE_ED */ 0X55,  /* small i with acute accent */
    /* ITOE_EE */ 0X56,  /* small i with circumflex accent */
    /* ITOE_EF */ 0X57,  /* small i with diaeresis */
/**/
    /* ITOE_F0 */ 0X8C,  /* small eth, Icelandic */
    /* ITOE_F1 */ 0X49,  /* small n with tilde */
    /* ITOE_F2 */ 0XCD,  /* small o with grave accent */
    /* ITOE_F3 */ 0XCE,  /* small o with acute accent */
    /* ITOE_F4 */ 0XCB,  /* small o with circumflex accent */
    /* ITOE_F5 */ 0XCF,  /* small o with tilde */
    /* ITOE_F6 */ 0XCC,  /* small o with diaeresis */
    /* ITOE_F7 */ 0XE1,  /* divide sign (dot over line over dot) */
    /* ITOE_F8 */ 0X70,  /* small o with slash */
    /* ITOE_F9 */ 0XDD,  /* small u with grave accent */
    /* ITOE_FA */ 0XDE,  /* small u with acute accent */
    /* ITOE_FB */ 0XDB,  /* small u with circumflex accent */
    /* ITOE_FC */ 0XDC,  /* small u with diaeresis */
    /* ITOE_FD */ 0X8D,  /* small y with acute accent */
    /* ITOE_FE */ 0X8E,  /* small thorn, Icelandic */
    /* ITOE_FF */ 0XDF}; /* small y diaeresis */
/************************************************************* 
*                                                            * 
*      New EBCDIC to ISO table : name  EBCASC                * 
*                                                            * 
**************************************************************/
unsigned char EBCASC[256]={
/*
                                  ISO name (with common usage names)
*/
    /* ETOI_00 */ 0X00,  /* NUL   null  */
    /* ETOI_01 */ 0X01,  /* SOH   start of heading           (Ctrl-A)  */
    /* ETOI_02 */ 0X02,  /* STX   start of text              (Ctrl-B)  */
    /* ETOI_03 */ 0X03,  /* ETX   end of text                (Ctrl-C)  */
    /* ETOI_04 */ 0X9C,  /* ...  */
    /* ETOI_05 */ 0X09,  /* HT    horizontal tabulation      (Ctrl-I)  */
    /* ETOI_06 */ 0X86,  /* ...  */
    /* ETOI_07 */ 0X7F,  /* DEL   delete (rubout,   DEL control char)  */
    /* ETOI_08 */ 0X97,  /* ...  */
    /* ETOI_09 */ 0X8D,  /* ...  */
    /* ETOI_0A */ 0X8E,  /* ...  */
    /* ETOI_0B */ 0X0B,  /* VT    vertical tabulation        (Ctrl-K)  */
    /* ETOI_0C */ 0X0C,  /* FF    form feed                  (Ctrl-L)  */
    /* ETOI_0D */ 0X0D,  /* CR    carriage return            (Ctrl-M)  */
    /* ETOI_0E */ 0X0E,  /* SO    shift-out                  (Ctrl-N)  */
    /* ETOI_0F */ 0X0F,  /* SI    shift-in                   (Ctrl-O)  */
/**/ 
    /* ETOI_10 */ 0X10,  /* DLE   data link escape           (Ctrl-P)  */
    /* ETOI_11 */ 0X11,  /* DC1   device control 1    (X-Off,   Ctrl-Q)  */
    /* ETOI_12 */ 0X12,  /* DC2   device control 2           (Ctrl-R)  */
    /* ETOI_13 */ 0X13,  /* DC3   device control 3     (X-On,   Ctrl-S)  */
    /* ETOI_14 */ 0X9D,  /* ...  */
    /* ETOI_15 */ 0X85,  /* ...  */
    /* ETOI_16 */ 0X08,  /* BS    backspace                  (Ctrl-H)  */
    /* ETOI_17 */ 0X87,  /* ...  */
    /* ETOI_18 */ 0X18,  /* CAN   cancel                     (Ctrl-X)  */
    /* ETOI_19 */ 0X19,  /* EM    end of medium              (Ctrl-Y)  */
    /* ETOI_1A */ 0X92,  /* ...  */
    /* ETOI_1B */ 0X8F,  /* ...  */
    /* ETOI_1C */ 0X1C,  /* FS    file separator  */
    /* ETOI_1D */ 0X1D,  /* GS    group separator  */
    /* ETOI_1E */ 0X1E,  /* RS    record separator  */
    /* ETOI_1F */ 0X1F,  /* US    unit separator  */
/**/ 
    /* ETOI_20 */ 0X80,  /* ...  */
    /* ETOI_21 */ 0X81,  /* ...  */
    /* ETOI_22 */ 0X82,  /* ...  */
    /* ETOI_23 */ 0X83,  /* ...  */
    /* ETOI_24 */ 0X84,  /* ...  */
    /* ETOI_25 */ 0X0A,  /* LF    line feed                  (Ctrl-J)  */
    /* ETOI_26 */ 0X17,  /* ETB   end of transmission block  (Ctrl-W)  */
    /* ETOI_27 */ 0X1B,  /* ESC   escape                     (Escape)  */
    /* ETOI_28 */ 0X88,  /* ...  */
    /* ETOI_29 */ 0X89,  /* ...  */
    /* ETOI_2A */ 0X8A,  /* ...  */
    /* ETOI_2B */ 0X8B,  /* ...  */
    /* ETOI_2C */ 0X8C,  /* ...  */
    /* ETOI_2D */ 0X05,  /* ENQ   enquiry                    (Ctrl-E)  */
    /* ETOI_2E */ 0X06,  /* ACK   acknowledge                (Ctrl-F)  */
    /* ETOI_2F */ 0X07,  /* BEL   bell                       (Ctrl-G)  */
/**/ 
    /* ETOI_30 */ 0X90,  /* ...  */
    /* ETOI_31 */ 0X91,  /* ...  */
    /* ETOI_32 */ 0X16,  /* SYN   synchronous idle           (Ctrl-V)  */
    /* ETOI_33 */ 0X93,  /* ...  */
    /* ETOI_34 */ 0X94,  /* ...  */
    /* ETOI_35 */ 0X95,  /* ...  */
    /* ETOI_36 */ 0X96,  /* ...  */
    /* ETOI_37 */ 0X04,  /* EOT   end of transmission        (Ctrl-D)  */
    /* ETOI_38 */ 0X98,  /* ...  */
    /* ETOI_39 */ 0X99,  /* ...  */
    /* ETOI_3A */ 0X9A,  /* ...  */
    /* ETOI_3B */ 0X9B,  /* ...  */
    /* ETOI_3C */ 0X14,  /* DC4   device control 4           (Ctrl-T)  */
    /* ETOI_3D */ 0X15,  /* NAK   negative acknowledge       (Ctrl-U)  */
    /* ETOI_3E */ 0X9E,  /* ...  */
    /* ETOI_3F */ 0X1A,  /* SUB   substitute character       (Ctrl-Z)  */
/**/ 
    /* ETOI_40 */ 0X20,  /* space (blank)  */
    /* ETOI_41 */ 0XA0,  /* no-break space  */
    /* ETOI_42 */ 0XE2,  /* small a with circumflex accent  */
    /* ETOI_43 */ 0XE4,  /* small a with diaeresis  */
    /* ETOI_44 */ 0XE0,  /* small a with grave accent  */
    /* ETOI_45 */ 0XE1,  /* small a with acute accent  */
    /* ETOI_46 */ 0XE3,  /* small a with tilde  */
    /* ETOI_47 */ 0XE5,  /* small a with ring above  */
    /* ETOI_48 */ 0XE7,  /* small c with cedilla  */
    /* ETOI_49 */ 0XF1,  /* small n with tilde  */
    /* ETOI_4A */ 0XA2,  /* cent sign  */
    /* ETOI_4B */ 0X2E,  /* period,   full stop  */
    /* ETOI_4C */ 0X3C,  /* less-than sign  */
    /* ETOI_4D */ 0X28,  /* left parenthesis  */
    /* ETOI_4E */ 0X2B,  /* plus sign  */
    /* ETOI_4F */ 0X7C,  /* vertical line (bar,  "or" sign)  */
/**/ 
    /* ETOI_50 */ 0X26,  /* ampersand (and sign)  */
    /* ETOI_51 */ 0XE9,  /* small e with acute accent  */
    /* ETOI_52 */ 0XEA,  /* small e with circumflex accent  */
    /* ETOI_53 */ 0XEB,  /* small e with diaeresis  */
    /* ETOI_54 */ 0XE8,  /* small e with grave accent  */
    /* ETOI_55 */ 0XED,  /* small i with acute accent  */
    /* ETOI_56 */ 0XEE,  /* small i with circumflex accent  */
    /* ETOI_57 */ 0XEF,  /* small i with diaeresis  */
    /* ETOI_58 */ 0XEC,  /* small i with grave accent  */
    /* ETOI_59 */ 0XDF,  /* small sharp s,   German  */
    /* ETOI_5A */ 0X21,  /* exclamation mark  */
    /* ETOI_5B */ 0X24,  /* dollar sign  */
    /* ETOI_5C */ 0X2A,  /* asterisk (star)  */
    /* ETOI_5D */ 0X29,  /* right parenthesis  */
    /* ETOI_5E */ 0X3B,  /* semicolon  */
    /* ETOI_5F */ 0XAC,  /* not sign  */
/**/ 
    /* ETOI_60 */ 0X2D,  /* minus sign or hyphen  */
    /* ETOI_61 */ 0X2F,  /* solidus (slash)  */
    /* ETOI_62 */ 0XC2,  /* capital A with circumflex accent  */
    /* ETOI_63 */ 0XC4,  /* capital A with diaeresis  */
    /* ETOI_64 */ 0XC0,  /* capital A with grave accent  */
    /* ETOI_65 */ 0XC1,  /* capital A with acute accent  */
    /* ETOI_66 */ 0XC3,  /* capital A with tilde  */
    /* ETOI_67 */ 0XC5,  /* capital A with ring  */
    /* ETOI_68 */ 0XC7,  /* capital C with cedilla  */
    /* ETOI_69 */ 0XD1,  /* capital N with tilde  */
    /* ETOI_6A */ 0XA6,  /* broken bar  */
    /* ETOI_6B */ 0X2C,  /* comma  */
    /* ETOI_6C */ 0X25,  /* percent sign  */
    /* ETOI_6D */ 0X5F,  /* low line (underscore)  */
    /* ETOI_6E */ 0X3E,  /* greater-than sign  */
    /* ETOI_6F */ 0X3F,  /* question mark  */
/**/ 
    /* ETOI_70 */ 0XF8,  /* small o with slash  */
    /* ETOI_71 */ 0XC9,  /* capital E with acute accent  */
    /* ETOI_72 */ 0XCA,  /* capital E with circumflex accent  */
    /* ETOI_73 */ 0XCB,  /* capital E with diaeresis  */
    /* ETOI_74 */ 0XC8,  /* capital E with grave accent  */
    /* ETOI_75 */ 0XCD,  /* capital I with acute accent  */
    /* ETOI_76 */ 0XCE,  /* capital I with circumflex accent  */
    /* ETOI_77 */ 0XCF,  /* capital I with diaeresis  */
    /* ETOI_78 */ 0XCC,  /* capital I with grave accent  */
    /* ETOI_79 */ 0X60,  /* grave accent  */
    /* ETOI_7A */ 0X3A,  /* colon  */
    /* ETOI_7B */ 0X23,  /* number sign (hash mark,   sharp sign)  */
    /* ETOI_7C */ 0X40,  /* commercial at  */
    /* ETOI_7D */ 0X27,  /* apostrophe (single quote)  */
    /* ETOI_7E */ 0X3D,  /* equals sign  */
    /* ETOI_7F */ 0X22,  /* quotation mark (double quote)  */
/**/ 
    /* ETOI_80 */ 0XD8,  /* capital O with slash  */
    /* ETOI_81 */ 0X61,  /* small a  */
    /* ETOI_82 */ 0X62,  /* small b  */
    /* ETOI_83 */ 0X63,  /* small c  */
    /* ETOI_84 */ 0X64,  /* small d  */
    /* ETOI_85 */ 0X65,  /* small e  */
    /* ETOI_86 */ 0X66,  /* small f  */
    /* ETOI_87 */ 0X67,  /* small g  */
    /* ETOI_88 */ 0X68,  /* small h  */
    /* ETOI_89 */ 0X69,  /* small i  */
    /* ETOI_8A */ 0XAB,  /* angle quotation mark left (<< mark)  */
    /* ETOI_8B */ 0XBB,  /* angle quotation mark right (>> mark)  */
    /* ETOI_8C */ 0XF0,  /* small eth,   Icelandic  */
    /* ETOI_8D */ 0XFD,  /* small y with acute accent  */
    /* ETOI_8E */ 0XFE,  /* small thorn,   Icelandic  */
    /* ETOI_8F */ 0XB1,  /* plus or minus sign  */
/**/ 
    /* ETOI_90 */ 0XB0,  /* degree sign  */
    /* ETOI_91 */ 0X6A,  /* small j  */
    /* ETOI_92 */ 0X6B,  /* small k  */
    /* ETOI_93 */ 0X6C,  /* small l  */
    /* ETOI_94 */ 0X6D,  /* small m  */
    /* ETOI_95 */ 0X6E,  /* small n  */
    /* ETOI_96 */ 0X6F,  /* small o  */
    /* ETOI_97 */ 0X70,  /* small p  */
    /* ETOI_98 */ 0X71,  /* small q  */
    /* ETOI_99 */ 0X72,  /* small r  */
    /* ETOI_9A */ 0XAA,  /* ordinal indicator feminine  */
    /* ETOI_9B */ 0XBA,  /* ordinal indicator,  masculine  */
    /* ETOI_9C */ 0XE6,  /* small ae dipthong  */
    /* ETOI_9D */ 0XB8,  /* cedilla  */
    /* ETOI_9E */ 0XC6,  /* capital AE dipthong  */
    /* ETOI_9F */ 0XA4,  /* currency sign (lozenge)  */
/**/ 
    /* ETOI_A0 */ 0XB5,  /* micro sign (small mu)  */
    /* ETOI_A1 */ 0X7E,  /* tilde (wavy line)  */
    /* ETOI_A2 */ 0X73,  /* small s  */
    /* ETOI_A3 */ 0X74,  /* small t  */
    /* ETOI_A4 */ 0X75,  /* small u  */
    /* ETOI_A5 */ 0X76,  /* small v  */
    /* ETOI_A6 */ 0X77,  /* small w  */
    /* ETOI_A7 */ 0X78,  /* small x  */
    /* ETOI_A8 */ 0X79,  /* small y  */
    /* ETOI_A9 */ 0X7A,  /* small z  */
    /* ETOI_AA */ 0XA1,  /* inverted exclamation mark  */
    /* ETOI_AB */ 0XBF,  /* inverted question mark  */
    /* ETOI_AC */ 0XD0,  /* capital D with stroke,   Icelandic eth  */
    /* ETOI_AD */ 0XDD,  /* capital Y with acute accent  */
    /* ETOI_AE */ 0XDE,  /* capital thorn,   Icelandic  */
    /* ETOI_AF */ 0XAE,  /* registered sign (circled capital R)  */
/**/ 
    /* ETOI_B0 */ 0X5E,  /* circumflex accent  */
    /* ETOI_B1 */ 0XA3,  /* pound sign (Sterling currency)  */
    /* ETOI_B2 */ 0XA5,  /* yen sign (Nipponese currency)  */
    /* ETOI_B3 */ 0XB7,  /* middle dot (scalar product)  */
    /* ETOI_B4 */ 0XA9,  /* copyright sign (circled capital C)  */
    /* ETOI_B5 */ 0XA7,  /* section sign (S-half-above-S sign)  */
    /* ETOI_B6 */ 0XB6,  /* pilcrow (paragraph,   double-barred P)  */
    /* ETOI_B7 */ 0XBC,  /* fraction one-quarter (1/4)  */
    /* ETOI_B8 */ 0XBD,  /* fraction one-half (1/2)  */
    /* ETOI_B9 */ 0XBE,  /* fraction three-quarters (3/4)  */
    /* ETOI_BA */ 0X5B,  /* left square bracket  */
    /* ETOI_BB */ 0X5D,  /* right square bracket  */
    /* ETOI_BC */ 0XAF,  /* macron  */
    /* ETOI_BD */ 0XA8,  /* diaeresis or umlaut  */
    /* ETOI_BE */ 0XB4,  /* acute accent  */
    /* ETOI_BF */ 0XD7,  /* multiply sign (vector product)  */
/**/
    /* ETOI_C0 */ 0X7B,  /* left curly bracket (left brace)  */
    /* ETOI_C1 */ 0X41,  /* capital A  */
    /* ETOI_C2 */ 0X42,  /* capital B  */
    /* ETOI_C3 */ 0X43,  /* capital C  */
    /* ETOI_C4 */ 0X44,  /* capital D  */
    /* ETOI_C5 */ 0X45,  /* capital E  */
    /* ETOI_C6 */ 0X46,  /* capital F  */
    /* ETOI_C7 */ 0X47,  /* capital G  */
    /* ETOI_C8 */ 0X48,  /* capital H  */
    /* ETOI_C9 */ 0X49,  /* capital I  */
    /* ETOI_CA */ 0XAD,  /* soft hyphen  */
    /* ETOI_CB */ 0XF4,  /* small o with circumflex accent  */
    /* ETOI_CC */ 0XF6,  /* small o with diaeresis  */
    /* ETOI_CD */ 0XF2,  /* small o with grave accent  */
    /* ETOI_CE */ 0XF3,  /* small o with acute accent  */
    /* ETOI_CF */ 0XF5,  /* small o with tilde  */
/**/
    /* ETOI_D0 */ 0X7D,  /* right curly bracket (right brace)  */
    /* ETOI_D1 */ 0X4A,  /* capital J  */
    /* ETOI_D2 */ 0X4B,  /* capital K  */
    /* ETOI_D3 */ 0X4C,  /* capital L  */
    /* ETOI_D4 */ 0X4D,  /* capital M  */
    /* ETOI_D5 */ 0X4E,  /* capital N  */
    /* ETOI_D6 */ 0X4F,  /* capital O  */
    /* ETOI_D7 */ 0X50,  /* capital P  */
    /* ETOI_D8 */ 0X51,  /* capital Q  */
    /* ETOI_D9 */ 0X52,  /* capital R  */
    /* ETOI_DA */ 0XB9,  /* superscript one  */
    /* ETOI_DB */ 0XFB,  /* small u with circumflex accent  */
    /* ETOI_DC */ 0XFC,  /* small u with diaeresis  */
    /* ETOI_DD */ 0XF9,  /* small u with grave accent  */
    /* ETOI_DE */ 0XFA,  /* small u with acute accent  */
    /* ETOI_DF */ 0XFF,  /* small y diaeresis  */
/**/ 
    /* ETOI_E0 */ 0X5C,  /* reverse solidus (backslash)  */
    /* ETOI_E1 */ 0XF7,  /* divide sign (dot over line over dot)  */
    /* ETOI_E2 */ 0X53,  /* capital S  */
    /* ETOI_E3 */ 0X54,  /* capital T  */
    /* ETOI_E4 */ 0X55,  /* capital U  */
    /* ETOI_E5 */ 0X56,  /* capital V  */
    /* ETOI_E6 */ 0X57,  /* capital W  */
    /* ETOI_E7 */ 0X58,  /* capital X  */
    /* ETOI_E8 */ 0X59,  /* capital Y  */
    /* ETOI_E9 */ 0X5A,  /* capital Z  */
    /* ETOI_EA */ 0XB2,  /* superscript two (squared)  */
    /* ETOI_EB */ 0XD4,  /* capital O with circumflex accent  */
    /* ETOI_EC */ 0XD6,  /* capital O with diaeresis  */
    /* ETOI_ED */ 0XD2,  /* capital O with grave accent  */
    /* ETOI_EE */ 0XD3,  /* capital O with acute accent  */
    /* ETOI_EF */ 0XD5,  /* capital O with tilde  */
/**/ 
    /* ETOI_F0 */ 0X30,  /* digit zero  */
    /* ETOI_F1 */ 0X31,  /* digit one  */
    /* ETOI_F2 */ 0X32,  /* digit two  */
    /* ETOI_F3 */ 0X33,  /* digit three  */
    /* ETOI_F4 */ 0X34,  /* digit four  */
    /* ETOI_F5 */ 0X35,  /* digit five  */
    /* ETOI_F6 */ 0X36,  /* digit six  */
    /* ETOI_F7 */ 0X37,  /* digit seven  */
    /* ETOI_F8 */ 0X38,  /* digit eight  */
    /* ETOI_F9 */ 0X39,  /* digit nine  */
    /* ETOI_FA */ 0XB3,  /* superscript three (cubed)  */
    /* ETOI_FB */ 0XDB,  /* capital U with circumflex accent  */
    /* ETOI_FC */ 0XDC,  /* capital U with diaeresis  */
    /* ETOI_FD */ 0XD9,  /* capital U with grave accent  */
    /* ETOI_FE */ 0XDA,  /* capital U with acute accent  */
    /* ETOI_FF */ 0X9F}; /* ...  */
/************************************************************* 
*                                                            * 
*      old MTS EBCDIC to ISO table : name  MTSASC            * 
*                                                            * 
**************************************************************/
unsigned char MTSASC[256]={
/*
                                  ISO name (with common usage names)
*/
  /* MTSTOI_00 */ 0X00,  /* NUL   null  */
  /* MTSTOI_01 */ 0X01,  /* SOH   start of heading           (Ctrl-A)  */
  /* MTSTOI_02 */ 0X02,  /* STX   start of text              (Ctrl-B)  */
  /* MTSTOI_03 */ 0X03,  /* ETX   end of text                (Ctrl-C)  */
  /* MTSTOI_04 */ 0X85,  /* ...  */
  /* MTSTOI_05 */ 0X09,  /* HT    horizontal tabulation      (Ctrl-I)  */
  /* MTSTOI_06 */ 0X86,  /* ...  */
  /* MTSTOI_07 */ 0X7F,  /* DEL   delete (rubout,   DEL control char)  */
  /* MTSTOI_08 */ 0X87,  /* ...  */
  /* MTSTOI_09 */ 0X88,  /* ...  */
  /* MTSTOI_0A */ 0X89,  /* ...  */
  /* MTSTOI_0B */ 0X0B,  /* VT    vertical tabulation        (Ctrl-K)  */
  /* MTSTOI_0C */ 0X0C,  /* FF    form feed                  (Ctrl-L)  */
  /* MTSTOI_0D */ 0X0D,  /* CR    carriage return            (Ctrl-M)  */
  /* MTSTOI_0E */ 0X0E,  /* SO    shift-out                  (Ctrl-N)  */
  /* MTSTOI_0F */ 0X0F,  /* SI    shift-in                   (Ctrl-O)  */
/**/ 
  /* MTSTOI_10 */ 0X10,  /* DLE   data link escape           (Ctrl-P)  */
  /* MTSTOI_11 */ 0X11,  /* DC1   device control 1    (X-Off,   Ctrl-Q)  */
  /* MTSTOI_12 */ 0X12,  /* DC2   device control 2           (Ctrl-R)  */
  /* MTSTOI_13 */ 0X13,  /* DC3   device control 3     (X-On,   Ctrl-S)  */
  /* MTSTOI_14 */ 0X8A,  /* ...  */
  /* MTSTOI_15 */ 0X81,  /* ...  */
  /* MTSTOI_16 */ 0X08,  /* BS    backspace                  (Ctrl-H)  */
  /* MTSTOI_17 */ 0X8B,  /* ...  */
  /* MTSTOI_18 */ 0X18,  /* CAN   cancel                     (Ctrl-X)  */
  /* MTSTOI_19 */ 0X19,  /* EM    end of medium              (Ctrl-Y)  */
  /* MTSTOI_1A */ 0X8C,  /* ...  */
  /* MTSTOI_1B */ 0X8D,  /* ...  */
  /* MTSTOI_1C */ 0X1C,  /* FS    file separator  */
  /* MTSTOI_1D */ 0X1D,  /* GS    group separator  */
  /* MTSTOI_1E */ 0X1E,  /* RS    record separator  */
  /* MTSTOI_1F */ 0X1F,  /* US    unit separator  */
/**/ 
  /* MTSTOI_20 */ 0X8E,  /* ...  */
  /* MTSTOI_21 */ 0X8F,  /* ...  */
  /* MTSTOI_22 */ 0X90,  /* ...  */
  /* MTSTOI_23 */ 0X91,  /* ...  */
  /* MTSTOI_24 */ 0X92,  /* ...  */
  /* MTSTOI_25 */ 0X0A,  /* LF    line feed                  (Ctrl-J)  */
  /* MTSTOI_26 */ 0X17,  /* ETB   end of transmission block  (Ctrl-W)  */
  /* MTSTOI_27 */ 0X1B,  /* ESC   escape                     (Escape)  */
  /* MTSTOI_28 */ 0X93,  /* ...  */
  /* MTSTOI_29 */ 0X94,  /* ...  */
  /* MTSTOI_2A */ 0X95,  /* ...  */
  /* MTSTOI_2B */ 0X96,  /* ...  */
  /* MTSTOI_2C */ 0X97,  /* ...  */
  /* MTSTOI_2D */ 0X05,  /* ENQ   enquiry                    (Ctrl-E)  */
  /* MTSTOI_2E */ 0X06,  /* ACK   acknowledge                (Ctrl-F)  */
  /* MTSTOI_2F */ 0X07,  /* BEL   bell                       (Ctrl-G)  */
/**/ 
  /* MTSTOI_30 */ 0X98,  /* ...  */
  /* MTSTOI_31 */ 0X99,  /* ...  */
  /* MTSTOI_32 */ 0X16,  /* SYN   synchronous idle           (Ctrl-V)  */
  /* MTSTOI_33 */ 0XA0,  /* no-break space  */
  /* MTSTOI_34 */ 0XA1,  /* inverted exclamation mark  */
  /* MTSTOI_35 */ 0XA2,  /* cent sign  */
  /* MTSTOI_36 */ 0XA3,  /* pound sign (Sterling currency)  */
  /* MTSTOI_37 */ 0X04,  /* EOT   end of transmission        (Ctrl-D)  */
  /* MTSTOI_38 */ 0XA4,  /* currency sign (lozenge)  */
  /* MTSTOI_39 */ 0XA5,  /* yen sign (Nipponese currency)  */
  /* MTSTOI_3A */ 0XA6,  /* broken bar  */
  /* MTSTOI_3B */ 0XA7,  /* section sign (S-half-above-S sign)  */
  /* MTSTOI_3C */ 0X14,  /* DC4   device control 4           (Ctrl-T)  */
  /* MTSTOI_3D */ 0X15,  /* NAK   negative acknowledge       (Ctrl-U)  */
  /* MTSTOI_3E */ 0XAC,  /* not sign  */
  /* MTSTOI_3F */ 0X1A,  /* SUB   substitute character       (Ctrl-Z)  */
/**/ 
  /* MTSTOI_40 */ 0X20,  /* space (blank)  */
  /* MTSTOI_41 */ 0XAE,  /* registered sign (circled capital R)  */
  /* MTSTOI_42 */ 0XBA,  /* ordinal indicator,  masculine  */
  /* MTSTOI_43 */ 0XBB,  /* angle quotation mark right (>> mark)  */
  /* MTSTOI_44 */ 0XBD,  /* fraction one-half (1/2)  */
  /* MTSTOI_45 */ 0XBF,  /* inverted question mark  */
  /* MTSTOI_46 */ 0XC0,  /* capital A with grave accent  */
  /* MTSTOI_47 */ 0XC1,  /* capital A with acute accent  */
  /* MTSTOI_48 */ 0XC2,  /* capital A with circumflex accent  */
  /* MTSTOI_49 */ 0XC3,  /* capital A with tilde  */
  /* MTSTOI_4A */ 0X80,  /* ...  */
  /* MTSTOI_4B */ 0X2E,  /* period,   full stop  */
  /* MTSTOI_4C */ 0X3C,  /* less-than sign  */
  /* MTSTOI_4D */ 0X28,  /* left parenthesis  */
  /* MTSTOI_4E */ 0X2B,  /* plus sign  */
  /* MTSTOI_4F */ 0X7C,  /* vertical line (bar,  "or" sign)  */
/**/ 
  /* MTSTOI_50 */ 0X26,  /* ampersand (and sign)  */
  /* MTSTOI_51 */ 0XC4,  /* capital A with diaeresis  */
  /* MTSTOI_52 */ 0XC5,  /* capital A with ring  */
  /* MTSTOI_53 */ 0XC6,  /* capital AE dipthong  */
  /* MTSTOI_54 */ 0XC7,  /* capital C with cedilla  */
  /* MTSTOI_55 */ 0XC8,  /* capital E with grave accent  */
  /* MTSTOI_56 */ 0XC9,  /* capital E with acute accent  */
  /* MTSTOI_57 */ 0XAF,  /* macron  */
  /* MTSTOI_58 */ 0XCA,  /* capital E with circumflex accent  */
  /* MTSTOI_59 */ 0XCB,  /* capital E with diaeresis  */
  /* MTSTOI_5A */ 0X21,  /* exclamation mark  */
  /* MTSTOI_5B */ 0X24,  /* dollar sign  */
  /* MTSTOI_5C */ 0X2A,  /* asterisk (star)  */
  /* MTSTOI_5D */ 0X29,  /* right parenthesis  */
  /* MTSTOI_5E */ 0X3B,  /* semicolon  */
  /* MTSTOI_5F */ 0X7E,  /* tilde (wavy line)  */
/**/ 
  /* MTSTOI_60 */ 0X2D,  /* minus sign or hyphen  */
  /* MTSTOI_61 */ 0X2F,  /* solidus (slash)  */
  /* MTSTOI_62 */ 0XCC,  /* capital I with grave accent  */
  /* MTSTOI_63 */ 0XCD,  /* capital I with acute accent  */
  /* MTSTOI_64 */ 0XCE,  /* capital I with circumflex accent  */
  /* MTSTOI_65 */ 0XCF,  /* capital I with diaeresis  */
  /* MTSTOI_66 */ 0XD0,  /* capital D with stroke,   Icelandic eth  */
  /* MTSTOI_67 */ 0XD1,  /* capital N with tilde  */
  /* MTSTOI_68 */ 0XD2,  /* capital O with grave accent  */
  /* MTSTOI_69 */ 0XD3,  /* capital O with acute accent  */
  /* MTSTOI_6A */ 0XD4,  /* capital O with circumflex accent  */
  /* MTSTOI_6B */ 0X2C,  /* comma  */
  /* MTSTOI_6C */ 0X25,  /* percent sign  */
  /* MTSTOI_6D */ 0X5F,  /* low line (underscore)  */
  /* MTSTOI_6E */ 0X3E,  /* greater-than sign  */
  /* MTSTOI_6F */ 0X3F,  /* question mark  */
/**/ 
  /* MTSTOI_70 */ 0XD5,  /* capital O with tilde  */
  /* MTSTOI_71 */ 0XD6,  /* capital O with diaeresis  */
  /* MTSTOI_72 */ 0XD7,  /* multiply sign (vector product)  */
  /* MTSTOI_73 */ 0XD8,  /* capital O with slash  */
  /* MTSTOI_74 */ 0XD9,  /* capital U with grave accent  */
  /* MTSTOI_75 */ 0XDA,  /* capital U with acute accent  */
  /* MTSTOI_76 */ 0XDB,  /* capital U with circumflex accent  */
  /* MTSTOI_77 */ 0XDC,  /* capital U with diaeresis  */
  /* MTSTOI_78 */ 0XDD,  /* capital Y with acute accent  */
  /* MTSTOI_79 */ 0XDE,  /* capital thorn,   Icelandic  */
  /* MTSTOI_7A */ 0X3A,  /* colon  */
  /* MTSTOI_7B */ 0X23,  /* number sign (hash mark,   sharp sign)  */
  /* MTSTOI_7C */ 0X40,  /* commercial at  */
  /* MTSTOI_7D */ 0X27,  /* apostrophe (single quote)  */
  /* MTSTOI_7E */ 0X3D,  /* equals sign  */
  /* MTSTOI_7F */ 0X22,  /* quotation mark (double quote)  */
/**/ 
  /* MTSTOI_80 */ 0XDF,  /* small sharp s,   German  */
  /* MTSTOI_81 */ 0X61,  /* small a  */
  /* MTSTOI_82 */ 0X62,  /* small b  */
  /* MTSTOI_83 */ 0X63,  /* small c  */
  /* MTSTOI_84 */ 0X64,  /* small d  */
  /* MTSTOI_85 */ 0X65,  /* small e  */
  /* MTSTOI_86 */ 0X66,  /* small f  */
  /* MTSTOI_87 */ 0X67,  /* small g  */
  /* MTSTOI_88 */ 0X68,  /* small h  */
  /* MTSTOI_89 */ 0X69,  /* small i  */
  /* MTSTOI_8A */ 0XE0,  /* small a with grave accent  */
  /* MTSTOI_8B */ 0X7B,  /* left curly bracket (left brace)  */
  /* MTSTOI_8C */ 0XBC,  /* fraction one-quarter (1/4)  */
  /* MTSTOI_8D */ 0XA8,  /* diaeresis or umlaut  */
  /* MTSTOI_8E */ 0XAB,  /* angle quotation mark left (<< mark)  */
  /* MTSTOI_8F */ 0X9A,  /* ...  */
/**/ 
  /* MTSTOI_90 */ 0XE1,  /* small a with acute accent  */
  /* MTSTOI_91 */ 0X6A,  /* small j  */
  /* MTSTOI_92 */ 0X6B,  /* small k  */
  /* MTSTOI_93 */ 0X6C,  /* small l  */
  /* MTSTOI_94 */ 0X6D,  /* small m  */
  /* MTSTOI_95 */ 0X6E,  /* small n  */
  /* MTSTOI_96 */ 0X6F,  /* small o  */
  /* MTSTOI_97 */ 0X70,  /* small p  */
  /* MTSTOI_98 */ 0X71,  /* small q  */
  /* MTSTOI_99 */ 0X72,  /* small r  */
  /* MTSTOI_9A */ 0X60,  /* grave accent  */
  /* MTSTOI_9B */ 0X7D,  /* right curly bracket (right brace)  */
  /* MTSTOI_9C */ 0X82,  /* ...  */
  /* MTSTOI_9D */ 0XA9,  /* copyright sign (circled capital C)  */
  /* MTSTOI_9E */ 0X84,  /* ...  */
  /* MTSTOI_9F */ 0X83,  /* ...  */
/**/ 
  /* MTSTOI_A0 */ 0XAD,  /* soft hyphen  */
  /* MTSTOI_A1 */ 0XEF,  /* small i with diaeresis  */
  /* MTSTOI_A2 */ 0x73,  /* small s  */
  /* MTSTOI_A3 */ 0X74,  /* small t  */
  /* MTSTOI_A4 */ 0X75,  /* small u  */
  /* MTSTOI_A5 */ 0X76,  /* small v  */
  /* MTSTOI_A6 */ 0X77,  /* small w  */
  /* MTSTOI_A7 */ 0X78,  /* small x  */
  /* MTSTOI_A8 */ 0X79,  /* small y  */
  /* MTSTOI_A9 */ 0X7A,  /* small z  */
  /* MTSTOI_AA */ 0X5E,  /* circumflex accent  */
  /* MTSTOI_AB */ 0X9B,  /* ...  */
  /* MTSTOI_AC */ 0X9D,  /* ...  */
  /* MTSTOI_AD */ 0X5B,  /* left square bracket  */
  /* MTSTOI_AE */ 0XBE,  /* fraction three-quarters (3/4)  */
  /* MTSTOI_AF */ 0XAA,  /* ordinal indicator feminine  */
/**/ 
  /* MTSTOI_B0 */ 0XB0,  /* degree sign  */
  /* MTSTOI_B1 */ 0XB1,  /* plus or minus sign  */
  /* MTSTOI_B2 */ 0XB2,  /* superscript two (squared)  */
  /* MTSTOI_B3 */ 0XB3,  /* superscript three (cubed)  */
  /* MTSTOI_B4 */ 0XB4,  /* acute accent  */
  /* MTSTOI_B5 */ 0XB5,  /* micro sign (small mu)  */
  /* MTSTOI_B6 */ 0XB6,  /* pilcrow (paragraph,   double-barred P)  */
  /* MTSTOI_B7 */ 0XB7,  /* middle dot (scalar product)  */
  /* MTSTOI_B8 */ 0XB8,  /* cedilla  */
  /* MTSTOI_B9 */ 0XB9,  /* superscript one  */
  /* MTSTOI_BA */ 0X5C,  /* reverse solidus (backslash)  */
  /* MTSTOI_BB */ 0X9C,  /* ...  */
  /* MTSTOI_BC */ 0X9E,  /* ...  */
  /* MTSTOI_BD */ 0X5D,  /* right square bracket  */
  /* MTSTOI_BE */ 0XFE,  /* small thorn,   Icelandic  */
  /* MTSTOI_BF */ 0X9F,  /* ...  */
/**/ 
  /* MTSTOI_C0 */ 0XE2,  /* small a with circumflex accent  */
  /* MTSTOI_C1 */ 0X41,  /* capital A  */
  /* MTSTOI_C2 */ 0X42,  /* capital B  */
  /* MTSTOI_C3 */ 0X43,  /* capital C  */
  /* MTSTOI_C4 */ 0X44,  /* capital D  */
  /* MTSTOI_C5 */ 0X45,  /* capital E  */
  /* MTSTOI_C6 */ 0X46,  /* capital F  */
  /* MTSTOI_C7 */ 0X47,  /* capital G  */
  /* MTSTOI_C8 */ 0X48,  /* capital H  */
  /* MTSTOI_C9 */ 0X49,  /* capital I  */
  /* MTSTOI_CA */ 0XE3,  /* small a with tilde  */
  /* MTSTOI_CB */ 0XE4,  /* small a with diaeresis  */
  /* MTSTOI_CC */ 0XE5,  /* small a with ring above  */
  /* MTSTOI_CD */ 0XE6,  /* small ae dipthong  */
  /* MTSTOI_CE */ 0XE7,  /* small c with cedilla  */
  /* MTSTOI_CF */ 0XE8,  /* small e with grave accent  */
/**/ 
  /* MTSTOI_D0 */ 0XE9,  /* small e with acute accent  */
  /* MTSTOI_D1 */ 0X4A,  /* capital J  */
  /* MTSTOI_D2 */ 0X4B,  /* capital K  */
  /* MTSTOI_D3 */ 0X4C,  /* capital L  */
  /* MTSTOI_D4 */ 0X4D,  /* capital M  */
  /* MTSTOI_D5 */ 0X4E,  /* capital N  */
  /* MTSTOI_D6 */ 0X4F,  /* capital O  */
  /* MTSTOI_D7 */ 0X50,  /* capital P  */
  /* MTSTOI_D8 */ 0X51,  /* capital Q  */
  /* MTSTOI_D9 */ 0X52,  /* capital R  */
  /* MTSTOI_DA */ 0XEA,  /* small e with circumflex accent  */
  /* MTSTOI_DB */ 0XEB,  /* small e with diaeresis  */
  /* MTSTOI_DC */ 0XEC,  /* small i with grave accent  */
  /* MTSTOI_DD */ 0xED,  /* small i with acute accent  */
  /* MTSTOI_DE */ 0xEE,  /* small i with circumflex accent  */
  /* MTSTOI_DF */ 0xF0,  /* small eth,   Icelandic  */
/**/ 
  /* MTSTOI_E0 */ 0xF1,  /* small n with tilde  */
  /* MTSTOI_E1 */ 0xF2,  /* small o with grave accent  */
  /* MTSTOI_E2 */ 0x53,  /* capital S  */
  /* MTSTOI_E3 */ 0x54,  /* capital T  */
  /* MTSTOI_E4 */ 0x55,  /* capital U  */
  /* MTSTOI_E5 */ 0x56,  /* capital V  */
  /* MTSTOI_E6 */ 0x57,  /* capital W  */
  /* MTSTOI_E7 */ 0x58,  /* capital X  */
  /* MTSTOI_E8 */ 0x59,  /* capital Y  */
  /* MTSTOI_E9 */ 0x5A,  /* capital Z  */
  /* MTSTOI_EA */ 0xF3,  /* small o with acute accent  */
  /* MTSTOI_EB */ 0xF4,  /* small o with circumflex accent  */
  /* MTSTOI_EC */ 0xF5,  /* small o with tilde  */
  /* MTSTOI_ED */ 0xF6,  /* small o with diaeresis  */
  /* MTSTOI_EE */ 0xF7,  /* divide sign (dot over line over dot)  */
  /* MTSTOI_EF */ 0xF8,  /* small o with slash  */
/**/ 
  /* MTSTOI_F0 */ 0x30,  /* digit zero  */
  /* MTSTOI_F1 */ 0x31,  /* digit one  */
  /* MTSTOI_F2 */ 0x32,  /* digit two  */
  /* MTSTOI_F3 */ 0x33,  /* digit three  */
  /* MTSTOI_F4 */ 0x34,  /* digit four  */
  /* MTSTOI_F5 */ 0x35,  /* digit five  */
  /* MTSTOI_F6 */ 0x36,  /* digit six  */
  /* MTSTOI_F7 */ 0x37,  /* digit seven  */
  /* MTSTOI_F8 */ 0x38,  /* digit eight  */
  /* MTSTOI_F9 */ 0x39,  /* digit nine  */
  /* MTSTOI_FA */ 0xF9,  /* small u with grave accent  */
  /* MTSTOI_FB */ 0xFA,  /* small u with acute accent  */
  /* MTSTOI_FC */ 0xFB,  /* small u with circumflex accent  */
  /* MTSTOI_FD */ 0xFC,  /* small u with diaeresis  */
  /* MTSTOI_FE */ 0xFD,  /* small y with acute accent  */
  /* MTSTOI_FF */ 0xFF}; /* small y diaeresis  */
/************************************************************* 
*                                                            * 
*      ISO to old MTS EBCDIC table : name  ASCMTS            * 
*                                                            * 
**************************************************************/
unsigned char ASCMTS[256]={
/*
                                  ISO name (with common usage names)
*/
  /* ITOMTS_00 */ 0X00,  /* NUL   null  */
  /* ITOMTS_01 */ 0X01,  /* SOH   start of heading           (Ctrl-A)  */
  /* ITOMTS_02 */ 0X02,  /* STX   start of text              (Ctrl-B)  */
  /* ITOMTS_03 */ 0X03,  /* ETX   end of text                (Ctrl-C)  */
  /* ITOMTS_04 */ 0X37,  /* EOT   end of transmission        (Ctrl-D)  */
  /* ITOMTS_05 */ 0X2D,  /* ENQ   enquiry                    (Ctrl-E)  */
  /* ITOMTS_06 */ 0X2E,  /* ACK   acknowledge                (Ctrl-F)  */
  /* ITOMTS_07 */ 0X2F,  /* BEL   bell                       (Ctrl-G)  */
  /* ITOMTS_08 */ 0X16,  /* BS    backspace                  (Ctrl-H)  */
  /* ITOMTS_09 */ 0X05,  /* HT    horizontal tabulation      (Ctrl-I)  */
  /* ITOMTS_0A */ 0X25,  /* LF    line feed                  (Ctrl-J)  */
  /* ITOMTS_0B */ 0X0B,  /* VT    vertical tabulation        (Ctrl-K)  */
  /* ITOMTS_0C */ 0X0C,  /* FF    form feed                  (Ctrl-L)  */
  /* ITOMTS_0D */ 0X0D,  /* CR    carriage return            (Ctrl-M)  */
  /* ITOMTS_0E */ 0X0E,  /* SO    shift-out                  (Ctrl-N)  */
  /* ITOMTS_0F */ 0X0F,  /* SI    shift-in                   (Ctrl-O)  */
/**/ 
  /* ITOMTS_10 */ 0X10,  /* DLE   data link escape           (Ctrl-P)  */
  /* ITOMTS_11 */ 0X11,  /* DC1   device control 1    (X-Off,   Ctrl-Q)  */
  /* ITOMTS_12 */ 0X12,  /* DC2   device control 2           (Ctrl-R)  */
  /* ITOMTS_13 */ 0X13,  /* DC3   device control 3     (X-On,   Ctrl-S)  */
  /* ITOMTS_14 */ 0X3C,  /* DC4   device control 4           (Ctrl-T)  */
  /* ITOMTS_15 */ 0X3D,  /* NAK   negative acknowledge       (Ctrl-U)  */
  /* ITOMTS_16 */ 0X32,  /* SYN   synchronous idle           (Ctrl-V)  */
  /* ITOMTS_17 */ 0X26,  /* ETB   end of transmission block  (Ctrl-W)  */
  /* ITOMTS_18 */ 0X18,  /* CAN   cancel                     (Ctrl-X)  */
  /* ITOMTS_19 */ 0X19,  /* EM    end of medium              (Ctrl-Y)  */
  /* ITOMTS_1A */ 0X3F,  /* SUB   substitute character       (Ctrl-Z)  */
  /* ITOMTS_1B */ 0X27,  /* ESC   escape                     (Escape)  */
  /* ITOMTS_1C */ 0X1C,  /* FS    file separator  */
  /* ITOMTS_1D */ 0X1D,  /* GS    group separator  */
  /* ITOMTS_1E */ 0X1E,  /* RS    record separator  */
  /* ITOMTS_1F */ 0X1F,  /* US    unit separator  */
/**/ 
  /* ITOMTS_20 */ 0X40,  /* space (blank)  */
  /* ITOMTS_21 */ 0X5A,  /* exclamation mark  */
  /* ITOMTS_22 */ 0X7F,  /* quotation mark (double quote)  */
  /* ITOMTS_23 */ 0X7B,  /* number sign (hash mark,   sharp sign)  */
  /* ITOMTS_24 */ 0X5B,  /* dollar sign  */
  /* ITOMTS_25 */ 0X6C,  /* percent sign  */
  /* ITOMTS_26 */ 0X50,  /* ampersand (and sign)  */
  /* ITOMTS_27 */ 0X7D,  /* apostrophe (single quote)  */
  /* ITOMTS_28 */ 0X4D,  /* left parenthesis  */
  /* ITOMTS_29 */ 0X5D,  /* right parenthesis  */
  /* ITOMTS_2A */ 0X5C,  /* asterisk (star)  */
  /* ITOMTS_2B */ 0X4E,  /* plus sign  */
  /* ITOMTS_2C */ 0X6B,  /* comma  */
  /* ITOMTS_2D */ 0X60,  /* minus sign or hyphen  */
  /* ITOMTS_2E */ 0X4B,  /* period,   full stop  */
  /* ITOMTS_2F */ 0X61,  /* solidus (slash)  */
/**/ 
  /* ITOMTS_30 */ 0XF0,  /* digit zero  */
  /* ITOMTS_31 */ 0XF1,  /* digit one  */
  /* ITOMTS_32 */ 0XF2,  /* digit two  */
  /* ITOMTS_33 */ 0XF3,  /* digit three  */
  /* ITOMTS_34 */ 0XF4,  /* digit four  */
  /* ITOMTS_35 */ 0XF5,  /* digit five  */
  /* ITOMTS_36 */ 0XF6,  /* digit six  */
  /* ITOMTS_37 */ 0XF7,  /* digit seven  */
  /* ITOMTS_38 */ 0XF8,  /* digit eight  */
  /* ITOMTS_39 */ 0XF9,  /* digit nine  */
  /* ITOMTS_3A */ 0X7A,  /* colon  */
  /* ITOMTS_3B */ 0X5E,  /* semicolon  */
  /* ITOMTS_3C */ 0X4C,  /* less-than sign  */
  /* ITOMTS_3D */ 0X7E,  /* equals sign  */
  /* ITOMTS_3E */ 0X6E,  /* greater-than sign  */
  /* ITOMTS_3F */ 0X6F,  /* question mark  */
/**/ 
  /* ITOMTS_40 */ 0X7C,  /* commercial at  */
  /* ITOMTS_41 */ 0XC1,  /* capital A  */
  /* ITOMTS_42 */ 0XC2,  /* capital B  */
  /* ITOMTS_43 */ 0XC3,  /* capital C  */
  /* ITOMTS_44 */ 0XC4,  /* capital D  */
  /* ITOMTS_45 */ 0XC5,  /* capital E  */
  /* ITOMTS_46 */ 0XC6,  /* capital F  */
  /* ITOMTS_47 */ 0XC7,  /* capital G  */
  /* ITOMTS_48 */ 0XC8,  /* capital H  */
  /* ITOMTS_49 */ 0XC9,  /* capital I  */
  /* ITOMTS_4A */ 0XD1,  /* capital J  */
  /* ITOMTS_4B */ 0XD2,  /* capital K  */
  /* ITOMTS_4C */ 0XD3,  /* capital L  */
  /* ITOMTS_4D */ 0XD4,  /* capital M  */
  /* ITOMTS_4E */ 0XD5,  /* capital N  */
  /* ITOMTS_4F */ 0XD6,  /* capital O  */
/**/ 
  /* ITOMTS_50 */ 0XD7,  /* capital P  */
  /* ITOMTS_51 */ 0XD8,  /* capital Q  */
  /* ITOMTS_52 */ 0XD9,  /* capital R  */
  /* ITOMTS_53 */ 0XE2,  /* capital S  */
  /* ITOMTS_54 */ 0XE3,  /* capital T  */
  /* ITOMTS_55 */ 0XE4,  /* capital U  */
  /* ITOMTS_56 */ 0XE5,  /* capital V  */
  /* ITOMTS_57 */ 0XE6,  /* capital W  */
  /* ITOMTS_58 */ 0XE7,  /* capital X  */
  /* ITOMTS_59 */ 0XE8,  /* capital Y  */
  /* ITOMTS_5A */ 0XE9,  /* capital Z  */
  /* ITOMTS_5B */ 0XAD,  /* left square bracket  */
  /* ITOMTS_5C */ 0XBA,  /* reverse solidus (backslash)  */
  /* ITOMTS_5D */ 0XBD,  /* right square bracket  */
  /* ITOMTS_5E */ 0XAA,  /* circumflex accent  */
  /* ITOMTS_5F */ 0X6D,  /* low line (underscore)  */
/**/ 
  /* ITOMTS_60 */ 0X9A,  /* grave accent  */
  /* ITOMTS_61 */ 0X81,  /* small a  */
  /* ITOMTS_62 */ 0X82,  /* small b  */
  /* ITOMTS_63 */ 0X83,  /* small c  */
  /* ITOMTS_64 */ 0X84,  /* small d  */
  /* ITOMTS_65 */ 0X85,  /* small e  */
  /* ITOMTS_66 */ 0X86,  /* small f  */
  /* ITOMTS_67 */ 0X87,  /* small g  */
  /* ITOMTS_68 */ 0X88,  /* small h  */
  /* ITOMTS_69 */ 0X89,  /* small i  */
  /* ITOMTS_6A */ 0X91,  /* small j  */
  /* ITOMTS_6B */ 0X92,  /* small k  */
  /* ITOMTS_6C */ 0X93,  /* small l  */
  /* ITOMTS_6D */ 0X94,  /* small m  */
  /* ITOMTS_6E */ 0X95,  /* small n  */
  /* ITOMTS_6F */ 0X96,  /* small o  */
/**/ 
  /* ITOMTS_70 */ 0X97,  /* small p  */
  /* ITOMTS_71 */ 0X98,  /* small q  */
  /* ITOMTS_72 */ 0X99,  /* small r  */
  /* ITOMTS_73 */ 0XA2,  /* small s  */
  /* ITOMTS_74 */ 0XA3,  /* small t  */
  /* ITOMTS_75 */ 0XA4,  /* small u  */
  /* ITOMTS_76 */ 0XA5,  /* small v  */
  /* ITOMTS_77 */ 0XA6,  /* small w  */
  /* ITOMTS_78 */ 0XA7,  /* small x  */
  /* ITOMTS_79 */ 0XA8,  /* small y  */
  /* ITOMTS_7A */ 0XA9,  /* small z  */
  /* ITOMTS_7B */ 0X8B,  /* left curly bracket (left brace)  */
  /* ITOMTS_7C */ 0X4F,  /* vertical line (bar,  "or" sign)  */
  /* ITOMTS_7D */ 0X9B,  /* right curly bracket (right brace)  */
  /* ITOMTS_7E */ 0X5F,  /* tilde (wavy line)  */
  /* ITOMTS_7F */ 0X07,  /* DEL   delete (rubout,   DEL control char)  */
/**/
  /* ITOMTS_80 */ 0X4A,  /* ...  */
  /* ITOMTS_81 */ 0X15,  /* ...  */
  /* ITOMTS_82 */ 0X9C,  /* ...  */
  /* ITOMTS_83 */ 0X9F,  /* ...  */
  /* ITOMTS_84 */ 0X9E,  /* ...  */
  /* ITOMTS_85 */ 0X04,  /* ...  */
  /* ITOMTS_86 */ 0X06,  /* ...  */
  /* ITOMTS_87 */ 0X08,  /* ...  */
  /* ITOMTS_88 */ 0X09,  /* ...  */
  /* ITOMTS_89 */ 0X0A,  /* ...  */
  /* ITOMTS_8A */ 0X14,  /* ...  */
  /* ITOMTS_8B */ 0X17,  /* ...  */
  /* ITOMTS_8C */ 0X1A,  /* ...  */
  /* ITOMTS_8D */ 0X1B,  /* ...  */
  /* ITOMTS_8E */ 0X20,  /* ...  */
  /* ITOMTS_8F */ 0X21,  /* ...  */
/**/
  /* ITOMTS_90 */ 0X22,  /* ...  */
  /* ITOMTS_91 */ 0X23,  /* ...  */
  /* ITOMTS_92 */ 0X24,  /* ...  */
  /* ITOMTS_93 */ 0X28,  /* ...  */
  /* ITOMTS_94 */ 0X29,  /* ...  */
  /* ITOMTS_95 */ 0X2A,  /* ...  */
  /* ITOMTS_96 */ 0X2B,  /* ...  */
  /* ITOMTS_97 */ 0X2C,  /* ...  */
  /* ITOMTS_98 */ 0X30,  /* ...  */
  /* ITOMTS_99 */ 0X31,  /* ...  */
  /* ITOMTS_9A */ 0X8F,  /* ...  */
  /* ITOMTS_9B */ 0XAB,  /* ...  */
  /* ITOMTS_9C */ 0XBB,  /* ...  */
  /* ITOMTS_9D */ 0XAC,  /* ...  */
  /* ITOMTS_9E */ 0XBC,  /* ...  */
  /* ITOMTS_9F */ 0XBF,  /* ...  */
/**/ 
  /* ITOMTS_A0 */ 0X33,  /* no-break space  */
  /* ITOMTS_A1 */ 0X34,  /* inverted exclamation mark  */
  /* ITOMTS_A2 */ 0X35,  /* cent sign  */
  /* ITOMTS_A3 */ 0X36,  /* pound sign (Sterling currency)  */
  /* ITOMTS_A4 */ 0X38,  /* currency sign (lozenge)  */
  /* ITOMTS_A5 */ 0X39,  /* yen sign (Nipponese currency)  */
  /* ITOMTS_A6 */ 0X3A,  /* broken bar  */
  /* ITOMTS_A7 */ 0X3B,  /* section sign (S-half-above-S sign)  */
  /* ITOMTS_A8 */ 0X8D,  /* diaeresis or umlaut  */
  /* ITOMTS_A9 */ 0X9D,  /* copyright sign (circled capital C)  */
  /* ITOMTS_AA */ 0XAF,  /* ordinal indicator feminine  */
  /* ITOMTS_AB */ 0X8E,  /* angle quotation mark left (<< mark)  */
  /* ITOMTS_AC */ 0X3E,  /* not sign  */
  /* ITOMTS_AD */ 0XA0,  /* soft hyphen  */
  /* ITOMTS_AE */ 0X41,  /* registered sign (circled capital R)  */
  /* ITOMTS_AF */ 0X57,  /* macron  */
/**/ 
  /* ITOMTS_B0 */ 0XB0,  /* degree sign  */
  /* ITOMTS_B1 */ 0XB1,  /* plus or minus sign  */
  /* ITOMTS_B2 */ 0XB2,  /* superscript two (squared)  */
  /* ITOMTS_B3 */ 0XB3,  /* superscript three (cubed)  */
  /* ITOMTS_B4 */ 0XB4,  /* acute accent  */
  /* ITOMTS_B5 */ 0XB5,  /* micro sign (small mu)  */
  /* ITOMTS_B6 */ 0XB6,  /* pilcrow (paragraph,   double-barred P)  */
  /* ITOMTS_B7 */ 0XB7,  /* middle dot (scalar product)  */
  /* ITOMTS_B8 */ 0XB8,  /* cedilla  */
  /* ITOMTS_B9 */ 0XB9,  /* superscript one  */
  /* ITOMTS_BA */ 0X42,  /* ordinal indicator,  masculine  */
  /* ITOMTS_BB */ 0X43,  /* angle quotation mark right (>> mark)  */
  /* ITOMTS_BC */ 0X8C,  /* fraction one-quarter (1/4)  */
  /* ITOMTS_BD */ 0X44,  /* fraction one-half (1/2)  */
  /* ITOMTS_BE */ 0XAE,  /* fraction three-quarters (3/4)  */
  /* ITOMTS_BF */ 0X45,  /* inverted question mark  */
/**/
  /* ITOMTS_C0 */ 0X46,  /* capital A with grave accent  */
  /* ITOMTS_C1 */ 0X47,  /* capital A with acute accent  */
  /* ITOMTS_C2 */ 0X48,  /* capital A with circumflex accent  */
  /* ITOMTS_C3 */ 0X49,  /* capital A with tilde  */
  /* ITOMTS_C4 */ 0X51,  /* capital A with diaeresis  */
  /* ITOMTS_C5 */ 0X52,  /* capital A with ring  */
  /* ITOMTS_C6 */ 0X53,  /* capital AE dipthong  */
  /* ITOMTS_C7 */ 0X54,  /* capital C with cedilla  */
  /* ITOMTS_C8 */ 0X55,  /* capital E with grave accent  */
  /* ITOMTS_C9 */ 0X56,  /* capital E with acute accent  */
  /* ITOMTS_CA */ 0X58,  /* capital E with circumflex accent  */
  /* ITOMTS_CB */ 0X59,  /* capital E with diaeresis  */
  /* ITOMTS_CC */ 0X62,  /* capital I with grave accent  */
  /* ITOMTS_CD */ 0X63,  /* capital I with acute accent  */
  /* ITOMTS_CE */ 0X64,  /* capital I with circumflex accent  */
  /* ITOMTS_CF */ 0X65,  /* capital I with diaeresis  */
/**/ 
  /* ITOMTS_D0 */ 0X66,  /* capital D with stroke,   Icelandic eth  */
  /* ITOMTS_D1 */ 0X67,  /* capital N with tilde  */
  /* ITOMTS_D2 */ 0X68,  /* capital O with grave accent  */
  /* ITOMTS_D3 */ 0X69,  /* capital O with acute accent  */
  /* ITOMTS_D4 */ 0X6A,  /* capital O with circumflex accent  */
  /* ITOMTS_D5 */ 0X70,  /* capital O with tilde  */
  /* ITOMTS_D6 */ 0X71,  /* capital O with diaeresis  */
  /* ITOMTS_D7 */ 0X72,  /* multiply sign (vector product)  */
  /* ITOMTS_D8 */ 0X73,  /* capital O with slash  */
  /* ITOMTS_D9 */ 0X74,  /* capital U with grave accent  */
  /* ITOMTS_DA */ 0X75,  /* capital U with acute accent  */
  /* ITOMTS_DB */ 0X76,  /* capital U with circumflex accent  */
  /* ITOMTS_DC */ 0X77,  /* capital U with diaeresis  */
  /* ITOMTS_DD */ 0X78,  /* capital Y with acute accent  */
  /* ITOMTS_DE */ 0X79,  /* capital thorn,   Icelandic  */
  /* ITOMTS_DF */ 0X80,  /* small sharp s,   German  */
/**/ 
  /* ITOMTS_E0 */ 0X8A,  /* small a with grave accent  */
  /* ITOMTS_E1 */ 0X90,  /* small a with acute accent  */
  /* ITOMTS_E2 */ 0XC0,  /* small a with circumflex accent  */
  /* ITOMTS_E3 */ 0XCA,  /* small a with tilde  */
  /* ITOMTS_E4 */ 0XCB,  /* small a with diaeresis  */
  /* ITOMTS_E5 */ 0XCC,  /* small a with ring above  */
  /* ITOMTS_E6 */ 0XCD,  /* small ae dipthong  */
  /* ITOMTS_E7 */ 0XCE,  /* small c with cedilla  */
  /* ITOMTS_E8 */ 0XCF,  /* small e with grave accent  */
  /* ITOMTS_E9 */ 0XD0,  /* small e with acute accent  */
  /* ITOMTS_EA */ 0XDA,  /* small e with circumflex accent  */
  /* ITOMTS_EB */ 0XDB,  /* small e with diaeresis  */
  /* ITOMTS_EC */ 0XDC,  /* small i with grave accent  */
  /* ITOMTS_ED */ 0XDD,  /* small i with acute accent  */
  /* ITOMTS_EE */ 0XDE,  /* small i with circumflex accent  */
  /* ITOMTS_EF */ 0XA1,  /* small i with diaeresis  */
/**/ 
  /* ITOMTS_F0 */ 0XDF,  /* small eth,   Icelandic  */
  /* ITOMTS_F1 */ 0XE0,  /* small n with tilde  */
  /* ITOMTS_F2 */ 0XE1,  /* small o with grave accent  */
  /* ITOMTS_F3 */ 0XEA,  /* small o with acute accent  */
  /* ITOMTS_F4 */ 0XEB,  /* small o with circumflex accent  */
  /* ITOMTS_F5 */ 0XEC,  /* small o with tilde  */
  /* ITOMTS_F6 */ 0XED,  /* small o with diaeresis  */
  /* ITOMTS_F7 */ 0XEE,  /* divide sign (dot over line over dot)  */
  /* ITOMTS_F8 */ 0XEF,  /* small o with slash  */
  /* ITOMTS_F9 */ 0XFA,  /* small u with grave accent  */
  /* ITOMTS_FA */ 0XFB,  /* small u with acute accent  */
  /* ITOMTS_FB */ 0XFC,  /* small u with circumflex accent  */
  /* ITOMTS_FC */ 0XFD,  /* small u with diaeresis  */
  /* ITOMTS_FD */ 0XFE,  /* small y with acute accent  */
  /* ITOMTS_FE */ 0XBE,  /* small thorn,   Icelandic  */
  /* ITOMTS_FF */ 0XFF}; /* small y diaeresis  */

#endif /* _TRANSLATE_H_ */
