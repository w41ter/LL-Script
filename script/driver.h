#ifndef __DRIVER_H__
#define __DRIVER_H__

namespace script
{
    class Driver
    {
    public:
        Driver() : filename(nullptr) {}
        bool parseArguments(int argc, char *argv[]);

    private:
        void usage();
        int command(int count, char *argv[]);

    public:
        bool dumpAST_ = false;
        bool dumpCFG_ = false;
        bool dumpQuad_ = false;
        bool dumpOpcode_ = false;
        bool optimized_ = false;

        const char *filename;
    };
}

#endif // !__DRIVER_H__
