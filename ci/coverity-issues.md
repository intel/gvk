
Uninitialized values
Missing deletes
Raw pointers (that Coverity flagged as potentially leaked) to `std::unique_ptr<>`/`std::vector<>`
Guard against self copy/move

Some (non exhaustive) examples...

o   Uninitialized variables

```
struct Widget
{
    uint32_t dataSize;
    uint8_t const* pData;
};
```

   Depending on how this structure is initialized, dataSize and pData could both easily be left uninitialized, reading from these variables results in undefined behavior.  Check out this blog post for some descriptions of the various gotchas in C++ initialization https://blog.tartanllama.xyz/initialization-is-bonkers/.  Initialization issues can be easily addressed by adding field initializers to struct/class declarations…

```
struct Widget
{
    uint32_t dataSize{};
    uint8_t const* pData{};
};
```

   The brace initializers cause the compiler generated default ctor to initialize the variables to their default values (0 and nullptr, respectively).

```
// Resulting compiler generated ctor
Widget()
    : dataSize{0}
    , pData{nullptr}
{
}
```

   This eliminates many opportunities for programmer error and ensures consistent initialization behavior.

o   Copy/assignment ctors not explicitly addressed for objects with non trivial dtors

```
class Whatsit
{
public:
    Whatsit(uint32_t value)
        : mValue{value}
    {
    }

private:
    uint32_t mValue{};
};

class Widget
{
public:
    Widget(uint32_t whatsitValue)
        : mpWhatsit{new Whatsit(whatsitValue)}
    {
    }

    ~Widget()
    {
        delete mpWhatsit;
    }

private:
    Whatsit* mpWhatsit{};
};
```

   In this example, we have a non trivial dtor (ie it manages resources).  This object isn’t safe because the compiler generated copy ctors will copy the object’s mpWhatsit pointer and every instance of the object will attempt to delete it when dtors run.  This can be addressed by implementing copy ctors that perform deep copies, move ctors, or simply disabling copies altogether by declaring the copy ctors deleted…

```
class Widget
{
public:
    Widget(uint32_t whatsitValue)
        : mpWhatsit{new Whatsit(whatsitValue)}
    {
    }

    ~Widget()
    {
        delete mpWhatsit;
    }

private:
    Whatsit* mpWhatsit{};
    Widget(Widget const&) = delete;
    Widget& operator=(Widget const&) = delete;
};
```

   Frequently I find that making an object non copyable is the best approach when it manages dynamic data, when these types of objects need to be stored in containers we can make them non copyable but movable…this will be illustrated in the next example.

o   Missing self copy/move guards

```
class Widget
{
public:
    Widget(uint32_t whatsitValue)
        : mpWhatsit{new Whatsit(whatsitValue)}
    {
    }

    Widget(Widget&& other)
    {
        *this = std::move(other);
    }

    Widget& operator=(Widget&& other)
    {
        mpWhatsit = std::move(other.mpWhatsit);
        other.mpWhatsit = nullptr;
        return *this;
    }

    ~Widget()
    {
        delete mpWhatsit;
    }

private:
    Whatsit* mpWhatsit{};
    Widget(Widget const&) = delete;
    Widget& operator=(Widget const&) = delete;
};
```

   In this example, we’ve declared our object to be non copyable and we’ve implemented move ctors so that we can store them in containers, but the move ctor doesn’t guard against a self move…if this == &other, when we return from the move assignment operator we’ll have leaked mpWhatsit.  This can be addressed by guarding against self copy/move…

```
Widget& operator=(Widget&& other)
{
    if (this != &other) {
        mpWhatsit = std::move(other.mpWhatsit);
        other.mpWhatsit = nullptr;
    }
    return *this;
}
```

o   Resource leaks due to missing calls to delete
   In most cases, std::unique_ptr<> should be preferred over raw pointers…it is one of the best examples of a “zero cost abstraction”…there’s really no downsides to using a std::unique_ptr<> over a raw pointer, and many upsides.  Consider the Widget class modified to use std::unique_ptr<>…

```
class Widget
{
public:
    Widget(uint32_t whatsitValue)
        : mupWhatsit{std::make_unique<Whatsit>(whatsitValue)}
    {
    }

private:
    std::unique_ptr<Whatsit> mupWhatsit;
    Widget(Widget const&) = delete;
    Widget& operator=(Widget const&) = delete;
}; 
```

   The crucial difference to note here is that we no longer need an explicitly defined dtor…the std::unique_ptr<> will clean up the memory it’s responsible for when its dtor runs as it goes out of scope…this removes a whole class of programmer errors related to memory management.
   std::vector<> (while not necessarily “zero cost”, its cost is low enough that it should be fine for most use cases) should be used similarly for dynamic arrays
