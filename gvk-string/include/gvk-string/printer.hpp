
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#pragma once

#include <cassert>
#include <iterator>
#include <iomanip>
#include <ostream>
#include <type_traits>
#include <utility>

namespace gvk {

class Printer;

/**
Prints a given object using a given Printer
@typename <ObjectType> The type of the object to print
@param [in] printer The Printer to print the given object with
@param [in] obj The object to print
*/
template <typename ObjectType>
void print(Printer& printer, const ObjectType& obj);

/**
Prints a specified bitmask
@typename <FlagBitsType> The enum type of the specified bitmask
@param [in] flags The std::underlying_type_t<FlagBitsType> value to print
*/
template <typename FlagBitsType>
void print(gvk::Printer& printer, std::underlying_type_t<FlagBitsType> flags);

/**
Provides an interface for writing objects composed of name/value pairs to a std::ostream
*/
class Printer
{
public:
    /**
    Bitmask specifying configuration options for printing
    */
    enum FlagBits
    {
        Formatted      = 1,
        FlushOnNewline = 1 << 1,
        EnumIdentifier = 1 << 2,
        EnumValue      = 1 << 3,
        Default        = Formatted | FlushOnNewline | EnumIdentifier | EnumValue
    };

    /**
    Bitmask of Printer::FlagBits
    */
    using Flags = uint32_t;

    /**
    Constructs an instance of Printer
    @param [in] ostrm The target std::ostream to print to
    @param [in] flags (optional = Printer::FlagBits::Default) Bitmask of Printer::FlagBits configuring printer output
    @param [in] tabCount (optional = 0) The tab count to begin printing at
    @param [in] tabSize (optional = 4) The tab size to use when printing tabs
    @param [in] pUserData (optional = nullptr) A pointer to user data to pass along while printing
    */
    inline Printer(std::ostream& ostrm, Flags flags = Default, int tabCount = 0, int tabSize = 4, const void* pUserData = nullptr)
        : mOstrm { ostrm }
        , mFlags { flags }
        , mTabCount { tabCount }
        , mTabSize { tabSize }
        , mpUserData { pUserData }
    {
    }

    /**
    Destroys this instance of Printer
    */
    inline virtual ~Printer()
    {
    }

    /**
    Gets this Printer object's Printer::FlagBits
    */
    inline Flags get_flags() const
    {
        return mFlags;
    }

    /**
    Prints an object using a given function
    @typename <PrintObjectFunctionType> The type of function to call to print the calling object's fields
    @param [in] printObject The function to call to print the calling object's fields
    */
    template <typename PrintObjectFunctionType>
    inline void print_object(PrintObjectFunctionType printObject)
    {
        print_object('{', printObject, '}');
    }

    /**
    Prints a named field
    @typename <ObjectType> The type of object to print
    @param [in] pName The name of the field to print
    @param [in] obj The value of the field to print
    */
    template <typename ObjectType>
    inline void print_field(const char* pName, const ObjectType& obj)
    {
        print_comma();
        print_name(pName);
        Printer printer(mOstrm, mFlags, mTabCount, mTabSize, mpUserData);
        print(printer, obj);
    }

    /**
    Prints an object if given a valid pointer, otherwise prints null
    @typename <ObjectType> The type of object of the given pointer
        @note ObjectType must have a gvk::print<>() specialization
    @param [in] pName The name of the object
    @param [in] pObj The pointer to print if valid
    */
    template <typename ObjectType>
    inline void print_pointer(const char* pName, const ObjectType* pObj)
    {
        print_comma();
        print_name(pName);
        if (pObj) {
            Printer printer(mOstrm, mFlags, mTabCount, mTabSize, mpUserData);
            print(printer, *pObj);
        } else {
            mOstrm << "null";
        }
    }

    /**
    Prints an array of objects if given a valid count and pointer, otherwise prints null
    @typename <CountType> The type of the given array's count
    @typename <ObjectType> The type of objects of the given array
    @typename <ProcessArrayElementFunctionType> The type of function used to process an element of the given array
    @param [in] pName The name of the array
    @param [in] count The number of objects in the array
    @param [in] pObjs The array of objects
    @param [in] processArrayElement The function used to process each array element
    */
    template <typename CountType, typename ObjectType, typename ProcessArrayElementFunctionType>
    inline void print_array(const char* pName, CountType count, const ObjectType* pObjs, ProcessArrayElementFunctionType processArrayElement)
    {
        print_comma();
        print_name(pName);
        if (count && pObjs) {
            print_object(
                '[',
                [&]()
                {
                    for (CountType i = 0; i < count; ++i) {
                        print_comma(i);
                        print_newline();
                        Printer printer(mOstrm, mFlags, mTabCount, mTabSize, mpUserData);
                        processArrayElement(printer, pObjs[i]);
                    }
                },
                ']'
            );
        } else {
            mOstrm << "null";
        }
    }

    /**
    Prints an array of objects if given a valid count and pointer, otherwise prints null
    @typename <CountType> The type of the given array's count
    @typename <ObjectType> The type of objects of the given array
        @note ObjectType must have a gvk::print<>() specialization
    @param [in] pName The name of the array
    @param [in] count The number of objects in the array
    @param [in] pObjs The array of objects
    */
    template <typename CountType, typename ObjectType>
    inline void print_array(const char* pName, CountType count, const ObjectType* pObjs)
    {
        if constexpr (std::is_pointer_v<CountType>) {
            print_array(
                pName, count ? *count : 0, pObjs,
                [](Printer& printer, const ObjectType& obj)
                {
                    print(printer, obj);
                }
            );
        } else {
            print_array(
                pName, count, pObjs,
                [](Printer& printer, const ObjectType& obj)
                {
                    print(printer, obj);
                }
            );
        }
    }

    /**
    Prints a collection of objects
    @typename <CollectionType> The type of the given collection
        @note CollectionType must provide iterable begin()/end() methods
    @typename <ProcessCollectionItrFunctionType> The type of function used to process a CollectionType::iterator_type
    @param [in] pName The name of the collection
    @param [in] collection The collection of objects
    @param [in] processCollectionItr The function used to process each iterator
    */
    template <typename CollectionType, typename ProcessCollectionItrFunctionType>
    inline void print_collection(
        const char* pName,
        const CollectionType& collection,
        ProcessCollectionItrFunctionType processCollectionItr
    )
    {
        print_comma();
        print_name(pName);
        print_object(
            '[',
            [&]()
            {
                int elementCount = 0;
                for (const auto& element : collection) {
                    print_comma(elementCount++);
                    print_newline();
                    Printer printer(mOstrm, mFlags, mTabCount, mTabSize, mpUserData);
                    print(printer, processCollectionItr(element));
                }
            },
            ']'
        );
    }

    /**
    Prints a collection of objects
    @typename <CollectionType> The type of the given collection
        @note CollectionType must provide iterable begin()/end() methods
    @param [in] pName The name of the collection
    @param [in] collection The collection of objects
    */
    template <typename CollectionType>
    inline void print_collection(const char* pName, const CollectionType& collection)
    {
        print_collection(pName, collection, [&](const auto& element) { return element; });
    }

    /**
    Prints an enum with a given identifier and value
    @typename <EnumType> The type of the enum to print
    @param [in] pIdentifier The enum identifier
    @param [in] obj The enum value
    */
    template <typename EnumType>
    inline void print_enum(const char* pIdentifier, EnumType value)
    {
        if (mFlags & Printer::EnumIdentifier && mFlags & Printer::EnumValue) {
            print_object(
                [&]()
                {
                    print_field("identifier", pIdentifier ? pIdentifier : "");
                    print_field("value", (std::underlying_type_t<EnumType>)value);
                }
            );
        } else if (mFlags & Printer::EnumIdentifier) {
            print(*this, pIdentifier ? pIdentifier : "");
        } else {
            print(*this, (std::underlying_type_t<EnumType>)value);
        }
    }

    /**
    Prints a specified bitmask
    @typename <FlagBitsType> The enum type of the specified bitmask
    @typename <FlagsType> The type of the given flags
        @note Usually std::underlying_type_t<FlagBitsType>
    @param [in] pIdentifier The identifier to print when Printer::FlagBits::EnumIdentifier is set
    @param [in] flags The value to print when Printer::FlagBits::EnumValue is set
    */
    template <typename FlagBitsType, typename FlagsType>
    inline void print_flags(const char* pIdentifier, FlagsType flags)
    {
        print_comma();
        print_name(pIdentifier);
        Printer printer(mOstrm, mFlags, mTabCount, mTabSize, mpUserData);
        print<FlagBitsType>(printer, flags);
    }

    /**
    Gets this Printer object's user data pointer
    @return This Printer Object's user data pointer
    */
    inline const void* get_user_data() const
    {
        return mpUserData;
    }

    /**
    Sets this Printer object's user data pointer
    @param pUserData The pointer to user data
        @note This pointer is not used by Printer; it's intended as a mechanism for user provided context while printing
    */
    inline void set_user_data(const void* pUserData)
    {
        mpUserData = pUserData;
    }

private:
    inline void print_name(const char* pName)
    {
        assert(pName);
        print_newline();
        mOstrm << "\"" << pName << "\""; mOstrm << ":"; print_whitespace(1);
    }

    template <typename IndexType>
    inline void print_comma(IndexType index)
    {
        if (index) {
            mOstrm << ",";
        }
    }

    inline void print_comma()
    {
        print_comma(mFieldCount++);
    }

    inline void print_whitespace(int count)
    {
        if (mFlags & Formatted) {
            std::fill_n(std::ostream_iterator<char>(mOstrm), count, ' ');
        }
    }

    inline void print_tab()
    {
        print_whitespace(mTabCount * mTabSize);
    }

    inline void print_newline()
    {
        if (mFlags & Formatted) {
            if (mFlags & FlushOnNewline) {
                mOstrm << std::endl;
            } else {
                mOstrm << "\n";
            }
            print_tab();
        }
    }

    template <typename PrintObjectFunctionType>
    inline void print_object(char openBrace, PrintObjectFunctionType printObject, char closeBrace)
    {
        mOstrm << openBrace;
        ++mTabCount;
        printObject();
        --mTabCount;
        print_newline();
        mOstrm << closeBrace;
    }

    std::ostream& mOstrm;
    Flags mFlags{ Default };
    int mTabCount{ };
    int mTabSize{ 4 };
    int mFieldCount{ };
    const void* mpUserData{ nullptr };

    template <typename ObjectType>
    friend void print(Printer&, const ObjectType&);
};

/**
Prints a given object using a given Printer
@typename <ObjectType> The type of the object to print
@param [in] printer The Printer to print the given object with
@param [in] obj The object to print
*/
template <typename ObjectType>
void print(Printer& printer, const ObjectType& obj)
{
    printer.mOstrm << obj;
}

/**
Prints a specified bitmask
@typename <FlagBitsType> The enum type of the specified bitmask
@param [in] flags The std::underlying_type_t<FlagBitsType> value to print
*/
template <typename FlagBitsType>
void print(gvk::Printer& printer, std::underlying_type_t<FlagBitsType> flags)
{
    (void)printer;
    (void)flags;
}

/**
Prints a given const char* using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] pStr The string to print
*/
template <>
inline void print<const char*>(Printer& printer, const char* const& pStr)
{
    if (pStr) {
        printer.mOstrm << "\"" << pStr << "\"";
    } else {
        printer.mOstrm << "null";
    }
}

/**
Prints a given std::string using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] str The string to print
*/
template <>
inline void print<std::string>(Printer& printer, const std::string& str)
{
    printer.mOstrm << "\"" << str << "\"";
}

/**
Prints a given bool using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] value The bool to print
*/
template <>
inline void print<bool>(Printer& printer, const bool& value)
{
    printer.mOstrm << (value ? "true" : "false");
}

/**
Prints a given float using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] value The float to print
*/
template <>
inline void print<float>(Printer& printer, const float& value)
{
    printer.mOstrm << std::scientific << std::setprecision(8) << value;
}

/**
Prints a given double using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] value The double to print
*/
template <>
inline void print<double>(Printer& printer, const double& value)
{
    printer.mOstrm << std::scientific << std::setprecision(16) << value;
}

/**
Prints a given long double using a given Printer
@param [in] printer The Printer to print the given object with
@param [in] value The long double to print
*/
template <>
inline void print<long double>(Printer& printer, const long double& value)
{
    printer.mOstrm << std::scientific << std::setprecision(32) << value;
}

} // namespace gvk
