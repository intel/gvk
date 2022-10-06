
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

#include "gvk/string.hpp"

#include "gtest/gtest.h"

static const std::string TheQuickBrownFox{ "The quick brown fox jumps over the lazy dog!" };

TEST(string, to_number)
{
    // Non number results in zero
    EXPECT_EQ(gvk::string::to_number<int>(TheQuickBrownFox), 0);
    EXPECT_EQ(gvk::string::to_number<float>(TheQuickBrownFox), 0.0f);

    // Integers are returned correctly
    EXPECT_EQ(gvk::string::to_number<int>("0.0"), 0);
    EXPECT_EQ(gvk::string::to_number<int>("1.0"), 1);
    EXPECT_EQ(gvk::string::to_number<int>("32"), 32);
    EXPECT_EQ(gvk::string::to_number<int>("-64"), -64);

    // Floats are returned correctly
    EXPECT_EQ(gvk::string::to_number<float>("0.0f"), 0.0f);
    EXPECT_EQ(gvk::string::to_number<float>("1.0f"), 1.0f);
    EXPECT_EQ(gvk::string::to_number<float>("3.14f"), 3.14f);
    EXPECT_EQ(gvk::string::to_number<float>("-3.14f"), -3.14f);
}

TEST(string, contains)
{
    // Successful true
    EXPECT_TRUE(gvk::string::contains(TheQuickBrownFox, "fox"));
    EXPECT_TRUE(gvk::string::contains(TheQuickBrownFox, "rown fox ju"));
    EXPECT_TRUE(gvk::string::contains(TheQuickBrownFox, "j"));
    EXPECT_TRUE(gvk::string::contains(TheQuickBrownFox, "!"));

    // Successful false
    EXPECT_FALSE(gvk::string::contains(TheQuickBrownFox, "bat"));
    EXPECT_FALSE(gvk::string::contains(TheQuickBrownFox, "7"));
    EXPECT_FALSE(gvk::string::contains(TheQuickBrownFox, "?"));

    // Empty str true
    EXPECT_TRUE(gvk::string::contains(TheQuickBrownFox, std::string()));
    EXPECT_TRUE(gvk::string::contains(std::string(), std::string()));

    // Empty str false
    EXPECT_FALSE(gvk::string::contains(std::string(), TheQuickBrownFox));
}

TEST(string, starts_with)
{
    // Successful true
    EXPECT_TRUE(gvk::string::starts_with(TheQuickBrownFox, "T"));
    EXPECT_TRUE(gvk::string::starts_with(TheQuickBrownFox, "The"));
    EXPECT_TRUE(gvk::string::starts_with(TheQuickBrownFox, "The quick"));
    EXPECT_TRUE(gvk::string::starts_with(TheQuickBrownFox, TheQuickBrownFox));

    // Successful false
    EXPECT_FALSE(gvk::string::starts_with(TheQuickBrownFox, "he quick brown fox"));
    EXPECT_FALSE(gvk::string::starts_with(TheQuickBrownFox, "the"));
    EXPECT_FALSE(gvk::string::starts_with(TheQuickBrownFox, "8"));

    // Empty str true
    EXPECT_TRUE(gvk::string::starts_with(TheQuickBrownFox, std::string()));
    EXPECT_TRUE(gvk::string::starts_with(std::string(), std::string()));

    // Empty str false
    EXPECT_FALSE(gvk::string::starts_with(std::string(), TheQuickBrownFox));
}

TEST(string, ends_with)
{
    // Successful true
    EXPECT_TRUE(gvk::string::ends_with(TheQuickBrownFox, "!"));
    EXPECT_TRUE(gvk::string::ends_with(TheQuickBrownFox, "dog!"));
    EXPECT_TRUE(gvk::string::ends_with(TheQuickBrownFox, "lazy dog!"));
    EXPECT_TRUE(gvk::string::ends_with(TheQuickBrownFox, TheQuickBrownFox));

    // Successful false
    EXPECT_FALSE(gvk::string::ends_with(TheQuickBrownFox, "he quick brown fox"));
    EXPECT_FALSE(gvk::string::ends_with(TheQuickBrownFox, "the"));
    EXPECT_FALSE(gvk::string::ends_with(TheQuickBrownFox, "8"));

    // Empty str true
    EXPECT_TRUE(gvk::string::ends_with(TheQuickBrownFox, std::string()));
    EXPECT_TRUE(gvk::string::ends_with(std::string(), std::string()));

    // Empty str false
    EXPECT_FALSE(gvk::string::ends_with(std::string(), TheQuickBrownFox));
}

TEST(string, replace)
{
    // Successful replace
    auto str = TheQuickBrownFox;
    str = gvk::string::replace(str, "!", ".");
    str = gvk::string::replace(str, "quick", "slow");
    str = gvk::string::replace(str, "jumps", "trips");
    str = gvk::string::replace(str, "lazy", "sleeping");
    EXPECT_EQ(str, "The slow brown fox trips over the sleeping dog.");

    // Unsuccessful replace
    str = TheQuickBrownFox;
    str = gvk::string::replace(str, "fox", "fox");
    str = gvk::string::replace(str, std::string(), "bird");
    str = gvk::string::replace(str, "cat", "dog");
    str = gvk::string::replace(str, "frog", std::string());
    EXPECT_EQ(str, TheQuickBrownFox);

    // Empty str replace
    str = std::string();
    str = gvk::string::replace(str, "!", ".");
    str = gvk::string::replace(str, "quick", "slow");
    str = gvk::string::replace(str, "jumps", "trips");
    str = gvk::string::replace(str, "lazy", "sleeping");
    EXPECT_EQ(str, std::string());

    // Successful multi Replacement
    str = gvk::string::replace(
        TheQuickBrownFox,
        {
            { "!", "." },
            { "quick", "slow" },
            { "jumps", "trips" },
            { "lazy", "sleeping" },
        }
    );
    EXPECT_EQ(str, "The slow brown fox trips over the sleeping dog.");

    // Unsuccessful multi Replacement
    str = gvk::string::replace(
        TheQuickBrownFox,
        {
            { "fox", "fox" },
            { std::string(), "bird" },
            { "cat", "dog" },
            { "frog", std::string() },
        }
    );
    EXPECT_EQ(str, TheQuickBrownFox);

    // Empty str multi Replacement
    str = gvk::string::replace(
        std::string(),
        {
            { "!", "." },
            { "quick", "slow" },
            { "jumps", "trips" },
            { "lazy", "sleeping" },
        }
    );
    EXPECT_EQ(str, std::string());
}

TEST(string, remove)
{
    // Successful remove
    auto str = TheQuickBrownFox;
    str = gvk::string::remove(str, "The ");
    str = gvk::string::remove(str, "!");
    str = gvk::string::remove(str, "brown ");
    str = gvk::string::remove(str, "lazy ");
    EXPECT_EQ(str, "quick fox jumps over the dog");

    // Unsuccessful remove
    str = TheQuickBrownFox;
    str = gvk::string::remove(str, "9");
    str = gvk::string::remove(str, "antelope");
    str = gvk::string::remove(str, "The  ");
    str = gvk::string::remove(str, "  fox  ");
    EXPECT_EQ(str, TheQuickBrownFox);
}

TEST(string, reduce_sequence)
{
    std::string str = "some\\ugly\\/\\//\\path\\with\\a/////broken\\\\extension.....ext";
    str = gvk::string::replace(str, "\\", "/");
    str = gvk::string::reduce_sequence(str, "/");
    str = gvk::string::reduce_sequence(str, ".");
    str = gvk::string::replace(str, "ugly", "nice");
    str = gvk::string::replace(str, "broken", "decent");
    EXPECT_EQ(str, "some/nice/path/with/a/decent/extension.ext");
}

TEST(string, scrub_path)
{
    std::string str = "some//file/\\path/with\\various\\//conventions.txt";
    EXPECT_EQ(gvk::string::scrub_path(str), "some/file/path/with/various/conventions.txt");
}

TEST(string, is_whitespace)
{
    // Successful true
    EXPECT_TRUE(gvk::string::is_whitespace(' '));
    EXPECT_TRUE(gvk::string::is_whitespace('\f'));
    EXPECT_TRUE(gvk::string::is_whitespace('\n'));
    EXPECT_TRUE(gvk::string::is_whitespace('\r'));
    EXPECT_TRUE(gvk::string::is_whitespace('\t'));
    EXPECT_TRUE(gvk::string::is_whitespace('\v'));
    EXPECT_TRUE(gvk::string::is_whitespace(" \f\n\r\t\v"));

    // Successful false
    EXPECT_FALSE(gvk::string::is_whitespace("0"));
    EXPECT_FALSE(gvk::string::is_whitespace(" \f\n\r  text \t\v"));
}


TEST(string, trim_leading_whitespace)
{
    auto str = "        " + TheQuickBrownFox + "        ";
    EXPECT_EQ(gvk::string::trim_leading_whitespace(str), TheQuickBrownFox + "        ");
}

TEST(string, trim_trailing_whitespace)
{
    auto str = "        " + TheQuickBrownFox + "        ";
    EXPECT_EQ(gvk::string::trim_trailing_whitespace(str), "        " + TheQuickBrownFox);
}

TEST(string, trim_whitespace)
{
    auto str = "        " + TheQuickBrownFox + "        ";
    EXPECT_EQ(gvk::string::trim_whitespace(str), TheQuickBrownFox);
}

TEST(string, is_upper)
{
    // Successful true
    EXPECT_TRUE(gvk::string::is_upper("Z"));
    EXPECT_TRUE(gvk::string::is_upper("THE"));

    // Successful false
    EXPECT_FALSE(gvk::string::is_upper("z"));
    EXPECT_FALSE(gvk::string::is_upper(TheQuickBrownFox));
}

TEST(string, to_upper)
{
    EXPECT_EQ(gvk::string::to_upper(TheQuickBrownFox), "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!");
}

TEST(string, is_lower)
{
    // Successful true
    EXPECT_TRUE(gvk::string::is_lower("z"));
    EXPECT_TRUE(gvk::string::is_lower("the"));

    // Successful false
    EXPECT_FALSE(gvk::string::is_lower("Z"));
    EXPECT_FALSE(gvk::string::is_lower(TheQuickBrownFox));
}

TEST(string, to_lower)
{
    EXPECT_EQ(gvk::string::to_lower("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!"), "the quick brown fox jumps over the lazy dog!");
}

TEST(string, split)
{
    const std::vector<std::string> Tokens { "The", "quick", "brown", "fox" };

    // Empty str
    EXPECT_TRUE(gvk::string::split(std::string(), " ").empty());

    // char delimiter
    EXPECT_EQ(gvk::string::split("The;quick;brown;fox", ";"), Tokens);

    // char delimiter (prefix)
    EXPECT_EQ(gvk::string::split(";The;quick;brown;fox", ";"), Tokens);

    // char delimiter (postfix)
    EXPECT_EQ(gvk::string::split("The;quick;brown;fox;", ";"), Tokens);

    // char delimiter (prefix and postfix)
    EXPECT_EQ(gvk::string::split(";The;quick;brown;fox;", ";"), Tokens);

    // string delimiter
    EXPECT_EQ(gvk::string::split("The COW quick COW brown COW fox COW ", " COW "), Tokens);
}

TEST(string, split_snake_case)
{
    const std::vector<std::string> Tokens { "the", "quick", "brown", "fox" };
    EXPECT_EQ(gvk::string::split_snake_case("the_quick_brown_fox"), Tokens);
}

TEST(string, split_camel_case)
{
    const std::vector<std::string> Tokens { "The", "Quick", "Brown", "FOX" };
    EXPECT_EQ(gvk::string::split_camel_case("TheQuickBrownFOX"), Tokens);
}
