
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

#include "gvk/printer.hpp"

#include "gtest/gtest.h"

class Foo final
{
public:
    int intValue{ };
    float floatValue{ };
};

class Bar final
{
public:
    const char* pName{ nullptr };
    size_t stringCount{ };
    const char* const* ppStrings{ nullptr };
    Foo foo;
};

class Baz final
{
public:
    const char* pName{ nullptr };
    bool active{ };
    size_t barCount{ };
    const Bar* pBars{ nullptr };
    const Foo* pFoo{ nullptr };
};

template <>
void gvk::print<Foo>(gvk::Printer& printer, const Foo& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("intValue", obj.intValue);
            printer.print_field("floatValue", obj.floatValue);
        }
    );
}

template <>
void gvk::print<Bar>(gvk::Printer& printer, const Bar& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("pName", obj.pName);
            printer.print_field("stringCount", obj.stringCount);
            printer.print_array("ppStrings", obj.stringCount, obj.ppStrings);
            printer.print_field("foo", obj.foo);
        }
    );
}

template <>
void gvk::print<Baz>(gvk::Printer& printer, const Baz& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_field("pName", obj.pName);
            printer.print_field("active", obj.active);
            printer.print_field("barCount", obj.barCount);
            printer.print_array("pBars", obj.barCount, obj.pBars);
            printer.print_pointer("pFoo", obj.pFoo);
        }
    );
}

TEST(Printer, print)
{
    Baz baz{ };
    EXPECT_EQ(gvk::to_string(baz, 0), R"({"pName":null,"active":false,"barCount":0,"pBars":null,"pFoo":null})");
    EXPECT_EQ(gvk::to_string(baz), R"({
    "pName": null,
    "active": false,
    "barCount": 0,
    "pBars": null,
    "pFoo": null
})");

    Foo foo{ };
    foo.intValue = 64;
    foo.floatValue = 3.14f;
    baz.pName = "Baz";
    baz.active = true;
    baz.pFoo = &foo;
    EXPECT_EQ(gvk::to_string(baz, 0), R"({"pName":"Baz","active":true,"barCount":0,"pBars":null,"pFoo":{"intValue":64,"floatValue":3.14000010e+00}})");
    EXPECT_EQ(gvk::to_string(baz), R"({
    "pName": "Baz",
    "active": true,
    "barCount": 0,
    "pBars": null,
    "pFoo": {
        "intValue": 64,
        "floatValue": 3.14000010e+00
    }
})");

    std::vector<const char*> strs{
        "The",
        "quick",
        nullptr,
        "brown",
        "fox",
    };
    std::vector<Bar> bars(3);
    bars[1].stringCount = strs.size();
    bars[1].ppStrings = strs.data();
    bars[2].foo = foo;
    baz.barCount = bars.size();
    baz.pBars = bars.data();
    EXPECT_EQ(gvk::to_string(baz, 0), R"({"pName":"Baz","active":true,"barCount":3,"pBars":[{"pName":null,"stringCount":0,"ppStrings":null,"foo":{"intValue":0,"floatValue":0.00000000e+00}},{"pName":null,"stringCount":5,"ppStrings":["The","quick",null,"brown","fox"],"foo":{"intValue":0,"floatValue":0.00000000e+00}},{"pName":null,"stringCount":0,"ppStrings":null,"foo":{"intValue":64,"floatValue":3.14000010e+00}}],"pFoo":{"intValue":64,"floatValue":3.14000010e+00}})");
    EXPECT_EQ(gvk::to_string(baz), R"({
    "pName": "Baz",
    "active": true,
    "barCount": 3,
    "pBars": [
        {
            "pName": null,
            "stringCount": 0,
            "ppStrings": null,
            "foo": {
                "intValue": 0,
                "floatValue": 0.00000000e+00
            }
        },
        {
            "pName": null,
            "stringCount": 5,
            "ppStrings": [
                "The",
                "quick",
                null,
                "brown",
                "fox"
            ],
            "foo": {
                "intValue": 0,
                "floatValue": 0.00000000e+00
            }
        },
        {
            "pName": null,
            "stringCount": 0,
            "ppStrings": null,
            "foo": {
                "intValue": 64,
                "floatValue": 3.14000010e+00
            }
        }
    ],
    "pFoo": {
        "intValue": 64,
        "floatValue": 3.14000010e+00
    }
})");
}

class Qux final
{
public:
    std::vector<int> ints;
    std::vector<float> floats;
};

class Quux final
{
public:
    std::set<std::string> strings;
    std::map<int, Qux> quxs;
};

template <>
void gvk::print<Qux>(gvk::Printer& printer, const Qux& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_collection("ints", obj.ints);
            printer.print_collection("floats", obj.floats);
        }
    );
}

template <>
void gvk::print<Quux>(gvk::Printer& printer, const Quux& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_collection("strings", obj.strings);
            printer.print_collection("quxs", obj.quxs, [](auto itr) { return itr.second; });
        }
    );
}

TEST(Printer, print_std_collections)
{
    Quux quux{ };
    EXPECT_EQ(gvk::to_string(quux, 0), R"({"strings":[],"quxs":[]})");
    EXPECT_EQ(gvk::to_string(quux), R"({
    "strings": [
    ],
    "quxs": [
    ]
})");

    Qux qux;
    quux.strings.insert("The");
    quux.strings.insert("quick");
    quux.strings.insert("brown");
    quux.strings.insert("fox");
    quux.quxs[32].floats.push_back(32.0f);
    quux.quxs[98].floats.push_back(98.6f);
    quux.quxs[212].floats.push_back(212.0f);
    EXPECT_EQ(gvk::to_string(quux, 0), R"({"strings":["The","brown","fox","quick"],"quxs":[{"ints":[],"floats":[3.20000000e+01]},{"ints":[],"floats":[9.85999985e+01]},{"ints":[],"floats":[2.12000000e+02]}]})");
    EXPECT_EQ(gvk::to_string(quux), R"({
    "strings": [
        "The",
        "brown",
        "fox",
        "quick"
    ],
    "quxs": [
        {
            "ints": [
            ],
            "floats": [
                3.20000000e+01
            ]
        },
        {
            "ints": [
            ],
            "floats": [
                9.85999985e+01
            ]
        },
        {
            "ints": [
            ],
            "floats": [
                2.12000000e+02
            ]
        }
    ]
})");
}

enum class ObjectType
{
    Undefined,
    Foo,
    Bar,
    Baz,
    Qux,
    Quxx,
};

template <>
void gvk::print<ObjectType>(gvk::Printer& printer, const ObjectType& value)
{
    switch (value) {
    case ObjectType::Undefined: printer.print_enum("ObjectType::Undefined", value); break;
    case ObjectType::Foo: printer.print_enum("ObjectType::Foo", value); break;
    case ObjectType::Bar: printer.print_enum("ObjectType::Bar", value); break;
    case ObjectType::Baz: printer.print_enum("ObjectType::Baz", value); break;
    case ObjectType::Qux: printer.print_enum("ObjectType::Qux", value); break;
    case ObjectType::Quxx: printer.print_enum("ObjectType::Quxx", value); break;
    default: assert(false);
    }
}

TEST(Printer, print_enum)
{
    EXPECT_EQ(gvk::to_string(ObjectType::Baz, 0), "3");
    EXPECT_EQ(gvk::to_string(ObjectType::Baz, gvk::Printer::EnumIdentifier), R"("ObjectType::Baz")");
    EXPECT_EQ(gvk::to_string(ObjectType::Baz, gvk::Printer::EnumValue), "3");
    EXPECT_EQ(gvk::to_string(ObjectType::Baz), R"({
    "identifier": "ObjectType::Baz",
    "value": 3
})");
}

class Frobnicator final
{
public:
    enum FlagBits
    {
        Fooify  = 1,
        Barify  = 1 << 1,
        Bazify  = 1 << 2,
        Quxify  = 1 << 3,
        Quxxify = 1 << 4,
    };

    using Flags = uint32_t;

    Flags flags{ };
};

template <> void
gvk::print<Frobnicator::FlagBits>(gvk::Printer& printer, std::underlying_type_t<Frobnicator::FlagBits> flags
)
{
    std::string flagsStr;
    if (printer.get_flags() & Printer::EnumIdentifier) {
        flagsStr = gvk::flags_to_string(flags,
            std::initializer_list<std::pair<Frobnicator::FlagBits, const char*>> {
                { Frobnicator::Fooify, "Frobnicator::Fooify" },
                { Frobnicator::Barify, "Frobnicator::Barify" },
                { Frobnicator::Bazify, "Frobnicator::Bazify" },
                { Frobnicator::Quxify, "Frobnicator::Quxify" },
                { Frobnicator::Quxxify, "Frobnicator::Quxxify" },
            }
        );
    }
    printer.print_enum(!flagsStr.empty() ? flagsStr.c_str() : nullptr, (Frobnicator::FlagBits)flags);
}

template <>
void gvk::print<Frobnicator>(gvk::Printer& printer, const Frobnicator& obj)
{
    printer.print_object(
        [&]()
        {
            printer.print_flags<Frobnicator::FlagBits>("flags", obj.flags);
        }
    );
}

TEST(Printer, print_flags)
{
    Frobnicator frobnicator{ };
    frobnicator.flags = Frobnicator::Fooify | Frobnicator::Bazify | Frobnicator::Quxxify;
    EXPECT_EQ(gvk::to_string(frobnicator), R"({
    "flags": {
        "identifier": "Frobnicator::Fooify|Frobnicator::Bazify|Frobnicator::Quxxify",
        "value": 21
    }
})");
}

TEST(Printer, to_hex_string)
{
    EXPECT_EQ(gvk::to_hex_string(3735928559), "0xdeadbeef");
}
