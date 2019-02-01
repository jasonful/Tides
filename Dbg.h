class Dbg
{
  public:
    #ifdef DEBUG
        Dbg() { Serial.begin(115200);}
        size_t print(const __FlashStringHelper *s)  { return Serial.print(s);}
        size_t print(const String &s) { return Serial.print(s);}
        size_t print(const char s[]) { return Serial.print(s);}
        size_t print(char c) { return Serial.print(c);}
        size_t print(unsigned char c, int base=DEC) { return Serial.print(c, base);}
        size_t print(int n, int base = DEC) { return Serial.print(n, base);}
        size_t print(unsigned int u, int base=DEC) { return Serial.print(u, base);}
        size_t print(long l, int base=DEC) { return Serial.print(l, base);}
        size_t print(unsigned long ul, int base=DEC) { return Serial.print(ul, base);}
        size_t print(double d, int digits=2) { return Serial.print(d, digits);}
        size_t print(const Printable& p) { return Serial.print(p);}

        size_t println(const __FlashStringHelper *s)  { return Serial.println(s);}
        size_t println(const String &s) { return Serial.println(s);}
        size_t println(const char s[]) { return Serial.println(s);}
        size_t println(char c) { return Serial.println(c);}
        size_t println(unsigned char c, int base=DEC) { return Serial.println(c, base);}
        size_t println(int n, int base=DEC) { return Serial.println(n, base);}
        size_t println(unsigned int u, int base=DEC) { return Serial.println(u, base);}
        size_t println(long l, int base=DEC) { return Serial.println(l, base);}
        size_t println(unsigned long ul, int base=DEC) { return Serial.println(ul, base);}
        size_t println(double d, int digits=2) { return Serial.println(d, digits);}
        size_t println(const Printable& p) { return Serial.println(p);}
        size_t println(void) { return Serial.println();}
    #else
        Dbg() {}
        size_t print(const __FlashStringHelper *s) {}
        size_t print(const String &s) {}
        size_t print(const char s[]) {}
        size_t print(char c) {}
        size_t print(unsigned char c, int base=DEC) {}
        size_t print(int n, int base = DEC) {}
        size_t print(unsigned int u, int base=DEC) {}
        size_t print(long l, int base=DEC) {}
        size_t print(unsigned long ul, int base=DEC) {}
        size_t print(double d, int digits=2) {}
        size_t print(const Printable& p) {}

        size_t println(const __FlashStringHelper *s)  {}
        size_t println(const String &s) {}
        size_t println(const char s[]) {}
        size_t println(char c) {}
        size_t println(unsigned char c, int base=DEC) {}
        size_t println(int n, int base=DEC) {}
        size_t println(unsigned int u, int base=DEC) {}
        size_t println(long l, int base=DEC) {}
        size_t println(unsigned long ul, int base=DEC) {}
        size_t println(double d, int digits=2) {}
        size_t println(const Printable& p) { }
        size_t println(void) {}
    #endif // DEBUG
};


