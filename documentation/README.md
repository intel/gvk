
# Documentation

When writing documentation, use Doxygen syntax/tags and follow [Microsoft's Docs style](https://docs.microsoft.com/en-us/contribute/style-quick-start).

# Examples

From `gvk-string/include/gvk/string.hpp`:

    /**
    Gets a value indicating whether or not a given string contains another given string
    @param [in] str The string to search
    @param [in] find The string to find
    @return Whether or not the given string contains the given find string
    */
    bool contains(const std::string& str, const std::string& find)

Note the use of the `@param` tags, they will generally be marked with one of the
following
- [in]
- [out]
- [in,out]

...to indicate how the parameter is used, in general `gvk` is `const` correct, so
it should be easy to understand from the docs how something should be used

From `gvk-string/include/printer.hpp`:

    /**
    Constructs an instance of Printer
    @param [in] ostrm The target std::ostream to print to
    @param [in] flags (optional = Printer::FlagBits::Default) Bitmask of Printer::FlagBits configuring printer output
    @param [in] tabCount (optional = 0) The tab count to begin printing at
    @param [in] tabSize (optional = 4) The tab size to use when printing tabs
    */
    inline Printer(std::ostream& ostrm, Flags flags = Default, int tabCount = 0, int tabSize = 4)

There are a copule of things to note in the example above.  The first is that
everything is fully qualified up to the current namespace.  For example, Printer
is in the `gvk` namespace...in general treat the docs like code in the current
namespace for the purposes of qualifying names.  The second is that optional
arguments are called out in the docs with an indicator after the parameter's
name of what the default value is.

From `gvk/include/gvk/render-target.hpp`:

    /**
    Gets this RenderTarget object's Framebuffer
    @return This RenderTarget object's Framebuffer
    */
    const Framebuffer& get_framebuffer() const;

Avoid appending "'s" directly to a type name, it breaks Doxygen hyperlinks.
