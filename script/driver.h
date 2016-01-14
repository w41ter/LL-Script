#ifndef __DRIVER_H__
#define __DRIVER_H__

namespace script
{
    class Driver
    {
        Driver() {}
    public:
        static Driver &instance()
        {
            static Driver inst;
            return inst;
        }

        void ParseArguments(int argc, char *argv[]);

    public:
        bool dumpAST_ = false;
        bool dumpIR_ = false;
        bool dumpOpcode_ = false;
        bool optimized_ = false;

        const char *filename;
    };
}

#endif // !__DRIVER_H__
