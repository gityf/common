#ifndef _DL_LIBRARY_H
#define _DL_LIBRARY_H

#include <string>
using std::string;
namespace common {

class Symbol;

/** @brief Shared library loader

    This class can be used to dynamically load shared libraries and
    resolve symbols from it. The example below shows how to retrieve
    the address of the function 'myProcedure' in library 'MyLibrary':

    @code
        Library shlib("MyLibrary.dll");
        void* procAddr = shlib["myProcedure"];

        typedef int (*MyProcType)();
        MyProcType proc =(MyProcType)procAddr;
        int result = proc();
    @endcode
*/
class Library
{
    public:
        /** @brief Default Constructor which does not load a library.
         */
        Library();

        /** @brief Loads a shared library.

             If a file could not be found at the given path, the path will be extended
             by the platform-specific shared library extension first and then also by the
             shared library prefix. If still no file can be found an exception is thrown.

             The library is loaded immediately.
        */
        explicit Library(const std::string& path);

        Library(const Library& other);

        Library& operator=(const Library& other);

        /** @brief The destructor unloads the shared library from memory.
         */
        ~Library();

        /** @brief Loads a shared library.

             If a file could not be found at the given path, the path will be extended
             by the platform-specific shared library extension first and then also by the
             shared library prefix. If still no file can be found an exception is thrown.
             Calling this method twice might close the previously loaded library.
        */
        Library& open(const std::string& path);

        void close();

        /** @brief Resolves the symbol \a symbol from the shared library
            Returns the address of the symbol or 0 if it was not found.
         */
        void* operator[](const char* symbol) const
        { return this->resolve(symbol); }

        /** @brief Resolves the symbol \a symbol from the shared library
            Returns the address of the symbol or 0 if it was not found.
         */
        void* resolve(const char* symbol) const;

        /** @brief Returns null if invalid
         */
        operator const void*() const;

        Symbol getSymbol(const char* symbol) const;

        /** @brief Returns true if invalid
         */
        bool operator!() const;

        /** @brief Returns the path to the shared library image
         */
        const std::string& path() const;

        /** @brief Returns the extension for shared libraries

            Returns ".so" on Linux, ".dll" on Windows.
        */
        static std::string suffix();

        /**  @brief Returns the prefix for shared libraries

             Returns "lib" on Linux, "" on Windows 
        */
        static std::string prefix();

    protected:
        void detach();

    private:
        //! @internal
        class LibraryImpl* _impl;

        //! @internal
        std::string _path;
};

/** @brief Symbol resolved from a shared library
*/
class Symbol
{
    public:
        Symbol()
        : _sym(0) { }

        Symbol(const Library& lib, void* sym)
        : _lib(lib), _sym(sym) { }

        void* sym() const
        { return _sym; }

        const Library& library() const
        { return _lib; }

        operator void*() const
        { return _sym; }

    private:
        Library _lib;
        void* _sym;
};

} // namespace common

#endif
