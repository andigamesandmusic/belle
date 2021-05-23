/* This file was authored by a third-party and is subject to the zlib license.

Project:  TinyXML2
License:  zlib
Homepage: https://github.com/leethomason/tinyxml2
Forked from: cf33e37d25346d108ef00b3bfa447b9a8f69382f (May 20, 2014)

--------------------------------------------------------------------------------

Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef PRIM_INCLUDE_MODTINYXML_H
#define PRIM_INCLUDE_MODTINYXML_H

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated"
  #pragma clang diagnostic ignored "-Wformat-nonliteral"
  #pragma clang diagnostic ignored "-Wimplicit-fallthrough"
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wpadded"
  #pragma clang diagnostic ignored "-Wsign-conversion"
  #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace PRIM_NAMESPACE { namespace meta {

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
#define PRIM_TINYXML_VISUALSTUDIO
#endif

#ifdef PRIM_TINYXML_VISUALSTUDIO
#include <stdarg.h>
inline int tixml_snprintf( char* buffer, unsigned int size,
    const char* format, ... )
{
    va_list va;
    va_start( va, format );
    int result = vsnprintf_s( buffer, size, _TRUNCATE, format, va );
    va_end( va );
    return result;
}
#define PRIM_TIXML_SNPRINTF tixml_snprintf
#define PRIM_TIXML_SSCANF   sscanf_s
#else
#define PRIM_TIXML_SNPRINTF snprintf
#define PRIM_TIXML_SSCANF   sscanf
#endif

namespace tinyxml2
{
class XMLDocument;
class XMLElement;
class XMLAttribute;
class XMLComment;
class XMLText;
class XMLDeclaration;
class XMLUnknown;
class XMLPrinter;

/*
    A class that wraps strings. Normally stores the start and end
    pointers into the XML file itself, and will apply normalization
    and entity translation if actually read. Can also store (and memory
    manage) a traditional char[]
*/
class StrPair
{
public:
    enum {
        NEEDS_ENTITY_PROCESSING         = 0x01,
        NEEDS_NEWLINE_NORMALIZATION     = 0x02,
        COLLAPSE_WHITESPACE                 = 0x04,

        TEXT_ELEMENT = NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
        TEXT_ELEMENT_LEAVE_ENTITIES     = NEEDS_NEWLINE_NORMALIZATION,
        ATTRIBUTE_NAME                      = 0,
        ATTRIBUTE_VALUE = NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
        ATTRIBUTE_VALUE_LEAVE_ENTITIES      = NEEDS_NEWLINE_NORMALIZATION,
        COMMENT                     = NEEDS_NEWLINE_NORMALIZATION
    };

    StrPair() : _flags( 0 ), _start( 0 ), _end( 0 ) {}
    ~StrPair();

    void Set( char* start, char* end, int flags ) {
        Reset();
        _start  = start;
        _end    = end;
        _flags  = flags | NEEDS_FLUSH;
    }

    const char* GetStr();

    bool Empty() const {
        return _start == _end;
    }

    void SetInternedStr( const char* str ) {
        Reset();
        _start = const_cast<char*>(str);
    }

    void SetStr( const char* str, int flags=0 );

    char* ParseText( char* in, const char* endTag, int strFlags );
    char* ParseName( char* in );

private:
    void Reset();
    void CollapseWhitespace();

    enum {
        NEEDS_FLUSH = 0x100,
        NEEDS_DELETE = 0x200
    };

    // After parsing, if *_end != 0, it can be set to zero.
    int     _flags;
    char*   _start;
    char*   _end;
};

/*
    A dynamic array of Plain Old Data. Doesn't support constructors, etc.
    Has a small initial memory pool, so that low or no usage will not
    cause a call to new/delete
*/
template <class T, int INIT>
class DynArray
{
public:
    DynArray< T, INIT >() {
        _mem = _pool;
        _allocated = INIT;
        _size = 0;
    }

    ~DynArray() {
        if ( _mem != _pool ) {
            delete [] _mem;
        }
    }

    void Clear() {
        _size = 0;
    }

    void Push( T t ) {
        EnsureCapacity( _size+1 );
        _mem[_size++] = t;
    }

    T* PushArr( int count ) {
        EnsureCapacity( _size+count );
        T* ret = &_mem[_size];
        _size += count;
        return ret;
    }

    T Pop() {
        return _mem[--_size];
    }

    void PopArr( int count ) {
        _size -= count;
    }

    bool Empty() const                  {
        return _size == 0;
    }

    T& operator[](int i)                {
        return _mem[i];
    }

    const T& operator[](int i) const    {
        return _mem[i];
    }

    const T& PeekTop() const                            {
        return _mem[ _size - 1];
    }

    int Size() const                    {
        return _size;
    }

    int Capacity() const                {
        return _allocated;
    }

    const T* Mem() const                {
        return _mem;
    }

    T* Mem()                            {
        return _mem;
    }

private:

    void EnsureCapacity( int cap ) {
        if ( cap > _allocated ) {
            int newAllocated = cap * 2;
            T* newMem = new T[newAllocated];
            Memory::MemCopy( newMem, _mem, (count)(sizeof(T)*_size));
            if ( _mem != _pool ) {
                delete [] _mem;
            }
            _mem = newMem;
            _allocated = newAllocated;
        }
    }

    T*  _mem;
    T   _pool[INIT];
    int _allocated;     // objects allocated
    int _size;          // number objects in use
};

/*
    Parent virtual class of a pool for fast allocation
    and deallocation of objects.
*/
class MemPool
{
public:
    MemPool() {}
    virtual ~MemPool() {}

    virtual int ItemSize() const = 0;
    virtual void* Alloc() = 0;
    virtual void Free( void* ) = 0;
    virtual void SetTracked() = 0;
};

/*
    Template child class to create pools of the correct type.
*/
template< int SIZE >
class MemPoolT : public MemPool
{
public:
    MemPoolT() : _root(0), _currentAllocs(0), _nAllocs(0), _maxAllocs(0),
        _nUntracked(0)    {}
    ~MemPoolT() {
        // Delete the blocks.
        for( int i=0; i<_blockPtrs.Size(); ++i ) {
            delete _blockPtrs[i];
        }
    }

    virtual int ItemSize() const    {
        return SIZE;
    }
    int CurrentAllocs() const       {
        return _currentAllocs;
    }

    virtual void* Alloc() {
        if ( !_root ) {
            // Need a new block.
            Block* block = new Block();
            _blockPtrs.Push( block );

            for( int i=0; i<COUNT-1; ++i ) {
                block->chunk[i].next = &block->chunk[i+1];
            }
            block->chunk[COUNT-1].next = 0;
            _root = block->chunk;
        }
        void* result = _root;
        _root = _root->next;

        ++_currentAllocs;
        if ( _currentAllocs > _maxAllocs ) {
            _maxAllocs = _currentAllocs;
        }
        _nAllocs++;
        _nUntracked++;
        return result;
    }
    virtual void Free( void* mem ) {
        if ( !mem ) {
            return;
        }
        --_currentAllocs;
        Chunk* chunk = (Chunk*)mem;
        chunk->next = _root;
        _root = chunk;
    }
    void Trace( const char* name ) {
        printf( "Mempool %s watermark=%d [%dk] current=%d "
            "size=%d nAlloc=%d blocks=%d\n",
                name, _maxAllocs, _maxAllocs*SIZE/1024, _currentAllocs, SIZE,
                _nAllocs, _blockPtrs.Size() );
    }

    void SetTracked() {
        _nUntracked--;
    }

    int Untracked() const {
        return _nUntracked;
    }

    // This number is perf sensitive.
    // The test file is large, 170k.
    // Release:     VS2010 gcc(no opt)
    //      1k:     4000
    //      2k:     4000
    //      4k:     3900    21000
    //      16k:    5200
    //      32k:    4300
    //      64k:    4000    21000
    enum { COUNT = (4*1024)/SIZE };

private:
    union Chunk {
        Chunk*  next;
        char    mem[SIZE];
    };
    struct Block {
        Chunk chunk[COUNT];
    };
    DynArray< Block*, 10 > _blockPtrs;
    Chunk* _root;

    int _currentAllocs;
    int _nAllocs;
    int _maxAllocs;
    int _nUntracked;
};

/**
    Implements the interface to the "Visitor pattern" (see the Accept() method.)
    If you call the Accept() method, it requires being passed a XMLVisitor
    class to handle callbacks. For nodes that contain other nodes
    (Document, Element)
    you will get called with a VisitEnter/VisitExit pair. Nodes that are always
    leafs
    are simply called with Visit().

    If you return 'true' from a Visit method, recursive parsing will continue.
    If you return
    false, <b>no children of this node or its siblings</b> will be visited.

    All flavors of Visit methods have a default implementation that returns
    'true' (continue
    visiting). You need to only override methods that are interesting to you.

    Generally Accept() is called on the XMLDocument, although all nodes support
    visiting.

    You should never change the document from a callback.

    @sa XMLNode::Accept()
*/
class XMLVisitor
{
public:
    virtual ~XMLVisitor() {}

    /// Visit a document.
    virtual bool VisitEnter( const XMLDocument& /*doc*/ )           {
        return true;
    }
    /// Visit a document.
    virtual bool VisitExit( const XMLDocument& /*doc*/ )            {
        return true;
    }

    /// Visit an element.
    virtual bool VisitEnter( const XMLElement& /*element*/, const XMLAttribute*
    /*firstAttribute*/ )    {
        return true;
    }
    /// Visit an element.
    virtual bool VisitExit( const XMLElement& /*element*/ )         {
        return true;
    }

    /// Visit a declaration.
    virtual bool Visit( const XMLDeclaration& /*declaration*/ )     {
        return true;
    }
    /// Visit a text node.
    virtual bool Visit( const XMLText& /*text*/ )                   {
        return true;
    }
    /// Visit a comment node.
    virtual bool Visit( const XMLComment& /*comment*/ )             {
        return true;
    }
    /// Visit an unknown node.
    virtual bool Visit( const XMLUnknown& /*unknown*/ )             {
        return true;
    }
};

/*
    Utility functionality.
*/
class XMLUtil
{
public:
    static const char* SkipWhiteSpace( const char* p );

    static char* SkipWhiteSpace( char* p );

    static bool IsWhiteSpace( char p );

    static bool IsNameStartChar( unsigned char ch );

    static bool IsNameChar( unsigned char ch );

    inline static bool StringEqual( const char* p, const char* q,
        int nChar=0x7fffffff )  {
        int n = 0;
        if ( p == q ) {
            return true;
        }
        while( *p && *q && *p == *q && n<nChar ) {
            ++p;
            ++q;
            ++n;
        }
        if ( (n == nChar) || ( *p == 0 && *q == 0 ) ) {
            return true;
        }
        return false;
    }

    inline static int IsUTF8Continuation( const char p ) {
        return p & 0x80;
    }

    static const char* ReadBOM( const char* p, bool* hasBOM );
    // p is the starting location,
    // the UTF-8 value of the entity will be placed in value, and length
    // filled in.
    static const char* GetCharacterRef( const char* p, char* value,
        int* length );
    static void ConvertUTF32ToUTF8( unsigned long input, char* output,
        int* length );

    // converts primitive types to strings
    static void ToStr( int v, char* buffer, int bufferSize );
    static void ToStr( unsigned v, char* buffer, int bufferSize );
    static void ToStr( bool v, char* buffer, int bufferSize );
    static void ToStr( float v, char* buffer, int bufferSize );
    static void ToStr( double v, char* buffer, int bufferSize );

    // converts strings to primitive types
    static bool ToInt( const char* str, int* value );
    static bool ToUnsigned( const char* str, unsigned* value );
    static bool ToBool( const char* str, bool* value );
    static bool ToFloat( const char* str, float* value );
    static bool ToDouble( const char* str, double* value );
};

/** XMLNode is a base class for every object that is in the
    XML Document Object Model (DOM), except XMLAttributes.
    Nodes have siblings, a parent, and children which can
    be navigated. A node is always in a XMLDocument.
    The type of a XMLNode can be queried, and it can
    be cast to its more defined type.

    A XMLDocument allocates memory for all its Nodes.
    When the XMLDocument gets deleted, all its Nodes
    will also be deleted.

    @verbatim
    A Document can contain: Element (container or leaf)
                            Comment (leaf)
                            Unknown (leaf)
                            Declaration( leaf )

    An Element can contain: Element (container or leaf)
                            Text    (leaf)
                            Attributes (not on tree)
                            Comment (leaf)
                            Unknown (leaf)

    @endverbatim
*/
class XMLNode
{
    friend class XMLDocument;
    friend class XMLElement;
public:

    /// Get the XMLDocument that owns this XMLNode.
    const XMLDocument* GetDocument() const  {
        return _document;
    }
    /// Get the XMLDocument that owns this XMLNode.
    XMLDocument* GetDocument()              {
        return _document;
    }

    /// Safely cast to an Element, or null.
    virtual XMLElement*     ToElement()     {
        return 0;
    }
    /// Safely cast to Text, or null.
    virtual XMLText*        ToText()        {
        return 0;
    }
    /// Safely cast to a Comment, or null.
    virtual XMLComment*     ToComment()     {
        return 0;
    }
    /// Safely cast to a Document, or null.
    virtual XMLDocument*    ToDocument()    {
        return 0;
    }
    /// Safely cast to a Declaration, or null.
    virtual XMLDeclaration* ToDeclaration() {
        return 0;
    }
    /// Safely cast to an Unknown, or null.
    virtual XMLUnknown*     ToUnknown()     {
        return 0;
    }

    virtual const XMLElement*       ToElement() const       {
        return 0;
    }
    virtual const XMLText*          ToText() const          {
        return 0;
    }
    virtual const XMLComment*       ToComment() const       {
        return 0;
    }
    virtual const XMLDocument*      ToDocument() const      {
        return 0;
    }
    virtual const XMLDeclaration*   ToDeclaration() const   {
        return 0;
    }
    virtual const XMLUnknown*       ToUnknown() const       {
        return 0;
    }

    /** The meaning of 'value' changes for the specific type.
        @verbatim
        Document:   empty
        Element:    name of the element
        Comment:    the comment text
        Unknown:    the tag contents
        Text:       the text string
        @endverbatim
    */
    const char* Value() const;

    /** Set the Value of an XML node.
        @sa Value()
    */
    void SetValue( const char* val, bool staticMem=false );

    /// Get the parent of this node on the DOM.
    const XMLNode*  Parent() const          {
        return _parent;
    }

    XMLNode* Parent()                       {
        return _parent;
    }

    /// Returns true if this node has no children.
    bool NoChildren() const                 {
        return !_firstChild;
    }

    /// Get the first child node, or null if none exists.
    const XMLNode*  FirstChild() const      {
        return _firstChild;
    }

    XMLNode*        FirstChild()            {
        return _firstChild;
    }

    /** Get the first child element, or optionally the first child
        element with the specified name.
    */
    const XMLElement* FirstChildElement( const char* value=0 ) const;

    XMLElement* FirstChildElement( const char* value=0 )    {
        return const_cast<XMLElement*>(const_cast<const XMLNode*>(
            this)->FirstChildElement( value ));
    }

    /// Get the last child node, or null if none exists.
    const XMLNode*  LastChild() const                       {
        return _lastChild;
    }

    XMLNode*        LastChild()                             {
        return const_cast<XMLNode*>(const_cast<const XMLNode*>(
            this)->LastChild() );
    }

    /** Get the last child element or optionally the last child
        element with the specified name.
    */
    const XMLElement* LastChildElement( const char* value=0 ) const;

    XMLElement* LastChildElement( const char* value=0 ) {
        return const_cast<XMLElement*>(const_cast<const XMLNode*>(
            this)->LastChildElement(value) );
    }

    /// Get the previous (left) sibling node of this node.
    const XMLNode*  PreviousSibling() const                 {
        return _prev;
    }

    XMLNode*    PreviousSibling()                           {
        return _prev;
    }

    /** Get the previous (left) sibling element of this node, with an optionally
    supplied name.*/
    const XMLElement*   PreviousSiblingElement( const char* value=0 ) const ;

    XMLElement* PreviousSiblingElement( const char* value=0 ) {
        return const_cast<XMLElement*>(const_cast<const XMLNode*>(
            this)->PreviousSiblingElement( value ) );
    }

    /// Get the next (right) sibling node of this node.
    const XMLNode*  NextSibling() const                     {
        return _next;
    }

    XMLNode*    NextSibling()                               {
        return _next;
    }

    /** Get the next (right) sibling element of this node, with an optionally
    supplied name.*/
    const XMLElement*   NextSiblingElement( const char* value=0 ) const;

    XMLElement* NextSiblingElement( const char* value=0 )   {
        return const_cast<XMLElement*>(const_cast<const XMLNode*>(
            this)->NextSiblingElement( value ) );
    }

    /**
        Add a child node as the last (right) child.
        If the child node is already part of the document,
        it is moved from its old location to the new location.
        Returns the addThis argument or 0 if the node does not
        belong to the same document.
    */
    XMLNode* InsertEndChild( XMLNode* addThis );

    XMLNode* LinkEndChild( XMLNode* addThis )   {
        return InsertEndChild( addThis );
    }
    /**
        Add a child node as the first (left) child.
        If the child node is already part of the document,
        it is moved from its old location to the new location.
        Returns the addThis argument or 0 if the node does not
        belong to the same document.
    */
    XMLNode* InsertFirstChild( XMLNode* addThis );
    /**
        Add a node after the specified child node.
        If the child node is already part of the document,
        it is moved from its old location to the new location.
        Returns the addThis argument or 0 if the afterThis node
        is not a child of this node, or if the node does not
        belong to the same document.
    */
    XMLNode* InsertAfterChild( XMLNode* afterThis, XMLNode* addThis );

    /**
        Delete all the children of this node.
    */
    void DeleteChildren();

    /**
        Delete a child of this node.
    */
    void DeleteChild( XMLNode* node );

    /**
        Make a copy of this node, but not its children.
        You may pass in a Document pointer that will be
        the owner of the new Node. If the 'document' is
        null, then the node returned will be allocated
        from the current Document. (this->GetDocument())

        Note: if called on a XMLDocument, this will return null.
    */
    virtual XMLNode* ShallowClone( XMLDocument* document ) const = 0;

    /**
        Test if 2 nodes are the same, but don't test children.
        The 2 nodes do not need to be in the same Document.

        Note: if called on a XMLDocument, this will return false.
    */
    virtual bool ShallowEqual( const XMLNode* compare ) const = 0;

    /** Accept a hierarchical visit of the nodes in the TinyXML-2 DOM.
        Every node in the
        XML tree will be conditionally visited and the host will be called back
        via the XMLVisitor interface.

        This is essentially a SAX interface for TinyXML-2. (Note however it
        doesn't re-parse
        the XML for the callbacks, so the performance of TinyXML-2 is unchanged
        by using this
        interface versus any other.)

        The interface has been based on ideas from:

        - http://www.saxproject.org/
        - http://c2.com/cgi/wiki?HierarchicalVisitorPattern

        Which are both good references for "visiting".

        An example of using Accept():
        @verbatim
        XMLPrinter printer;
        tinyxmlDoc.Accept( &printer );
        const char* xmlcstr = printer.CStr();
        @endverbatim
    */
    virtual bool Accept( XMLVisitor* visitor ) const = 0;

    // internal
    virtual char* ParseDeep( char*, StrPair* );

protected:
    XMLNode( XMLDocument* );
    virtual ~XMLNode();
    XMLNode( const XMLNode& );  // not supported
    XMLNode& operator=( const XMLNode& );   // not supported

    XMLDocument*    _document;
    XMLNode*        _parent;
    mutable StrPair _value;

    XMLNode*        _firstChild;
    XMLNode*        _lastChild;

    XMLNode*        _prev;
    XMLNode*        _next;

private:
    MemPool*        _memPool;
    void Unlink( XMLNode* child );
};

/** XML text.

    Note that a text node can have child element nodes, for example:
    @verbatim
    <root>This is <b>bold</b></root>
    @endverbatim

    A text node can have 2 ways to output the next. "normal" output
    and CDATA. It will default to the mode it was parsed from the XML file and
    you generally want to leave it alone, but you can change the output mode
    with
    SetCData() and query it with CData().
*/
class XMLText : public XMLNode
{
    friend class XMLBase;
    friend class XMLDocument;
public:
    virtual bool Accept( XMLVisitor* visitor ) const;

    virtual XMLText* ToText()           {
        return this;
    }
    virtual const XMLText* ToText() const   {
        return this;
    }

    /// Declare whether this should be CDATA or standard text.
    void SetCData( bool isCData )           {
        _isCData = isCData;
    }
    /// Returns true if this is a CDATA text element.
    bool CData() const                      {
        return _isCData;
    }

    char* ParseDeep( char*, StrPair* endTag );
    virtual XMLNode* ShallowClone( XMLDocument* document ) const;
    virtual bool ShallowEqual( const XMLNode* compare ) const;

protected:
    XMLText( XMLDocument* doc ) : XMLNode( doc ), _isCData( false ) {}
    virtual ~XMLText()                                              {}
    XMLText( const XMLText& );  // not supported
    XMLText& operator=( const XMLText& );   // not supported

private:
    bool _isCData;
};

/** An XML Comment. */
class XMLComment : public XMLNode
{
    friend class XMLDocument;
public:
    virtual XMLComment* ToComment()                 {
        return this;
    }
    virtual const XMLComment* ToComment() const     {
        return this;
    }

    virtual bool Accept( XMLVisitor* visitor ) const;

    char* ParseDeep( char*, StrPair* endTag );
    virtual XMLNode* ShallowClone( XMLDocument* document ) const;
    virtual bool ShallowEqual( const XMLNode* compare ) const;

protected:
    XMLComment( XMLDocument* doc );
    virtual ~XMLComment();
    XMLComment( const XMLComment& );    // not supported
    XMLComment& operator=( const XMLComment& ); // not supported

private:
};

/** In correct XML the declaration is the first entry in the file.
    @verbatim
        <?xml version="1.0" standalone="yes"?>
    @endverbatim

    TinyXML-2 will happily read or write files without a declaration,
    however.

    The text of the declaration isn't interpreted. It is parsed
    and written as a string.
*/
class XMLDeclaration : public XMLNode
{
    friend class XMLDocument;
public:
    virtual XMLDeclaration* ToDeclaration()                 {
        return this;
    }
    virtual const XMLDeclaration* ToDeclaration() const     {
        return this;
    }

    virtual bool Accept( XMLVisitor* visitor ) const;

    char* ParseDeep( char*, StrPair* endTag );
    virtual XMLNode* ShallowClone( XMLDocument* document ) const;
    virtual bool ShallowEqual( const XMLNode* compare ) const;

protected:
    XMLDeclaration( XMLDocument* doc );
    virtual ~XMLDeclaration();
    XMLDeclaration( const XMLDeclaration& );    // not supported
    XMLDeclaration& operator=( const XMLDeclaration& ); // not supported
};

/** Any tag that TinyXML-2 doesn't recognize is saved as an
    unknown. It is a tag of text, but should not be modified.
    It will be written back to the XML, unchanged, when the file
    is saved.

    DTD tags get thrown into XMLUnknowns.
*/
class XMLUnknown : public XMLNode
{
    friend class XMLDocument;
public:
    virtual XMLUnknown* ToUnknown()                 {
        return this;
    }
    virtual const XMLUnknown* ToUnknown() const     {
        return this;
    }

    virtual bool Accept( XMLVisitor* visitor ) const;

    char* ParseDeep( char*, StrPair* endTag );
    virtual XMLNode* ShallowClone( XMLDocument* document ) const;
    virtual bool ShallowEqual( const XMLNode* compare ) const;

protected:
    XMLUnknown( XMLDocument* doc );
    virtual ~XMLUnknown();
    XMLUnknown( const XMLUnknown& );    // not supported
    XMLUnknown& operator=( const XMLUnknown& ); // not supported
};

enum XMLError {
    XML_NO_ERROR = 0,
    XML_SUCCESS = 0,

    XML_NO_ATTRIBUTE,
    XML_WRONG_ATTRIBUTE_TYPE,

    XML_ERROR_FILE_NOT_FOUND,
    XML_ERROR_FILE_COULD_NOT_BE_OPENED,
    XML_ERROR_FILE_READ_ERROR,
    XML_ERROR_ELEMENT_MISMATCH,
    XML_ERROR_PARSING_ELEMENT,
    XML_ERROR_PARSING_ATTRIBUTE,
    XML_ERROR_IDENTIFYING_TAG,
    XML_ERROR_PARSING_TEXT,
    XML_ERROR_PARSING_CDATA,
    XML_ERROR_PARSING_COMMENT,
    XML_ERROR_PARSING_DECLARATION,
    XML_ERROR_PARSING_UNKNOWN,
    XML_ERROR_EMPTY_DOCUMENT,
    XML_ERROR_MISMATCHED_ELEMENT,
    XML_ERROR_PARSING,

    XML_CAN_NOT_CONVERT_TEXT,
    XML_NO_TEXT_NODE
};

/** An attribute is a name-value pair. Elements have an arbitrary
    number of attributes, each with a unique name.

    @note The attributes are not XMLNodes. You may only query the
    Next() attribute in a list.
*/
class XMLAttribute
{
    friend class XMLElement;
public:
    /// The name of the attribute.
    const char* Name() const;

    /// The value of the attribute.
    const char* Value() const;

    /// The next attribute in the list.
    const XMLAttribute* Next() const {
        return _next;
    }

    /** IntValue interprets the attribute as an integer, and returns the value.
        If the value isn't an integer, 0 will be returned. There is no error
        checking;
        use QueryIntValue() if you need error checking.
    */
    int      IntValue() const               {
        int i=0;
        QueryIntValue( &i );
        return i;
    }
    /// Query as an unsigned integer. See IntValue()
    unsigned UnsignedValue() const          {
        unsigned i=0;
        QueryUnsignedValue( &i );
        return i;
    }
    /// Query as a boolean. See IntValue()
    bool     BoolValue() const              {
        bool b=false;
        QueryBoolValue( &b );
        return b;
    }
    /// Query as a double. See IntValue()
    double   DoubleValue() const            {
        double d=0;
        QueryDoubleValue( &d );
        return d;
    }
    /// Query as a float. See IntValue()
    float    FloatValue() const             {
        float f=0;
        QueryFloatValue( &f );
        return f;
    }

    /** QueryIntValue interprets the attribute as an integer, and returns the
        value
        in the provided parameter. The function will return XML_NO_ERROR on
        success,
        and XML_WRONG_ATTRIBUTE_TYPE if the conversion is not successful.
    */
    XMLError QueryIntValue( int* value ) const;
    /// See QueryIntValue
    XMLError QueryUnsignedValue( unsigned int* value ) const;
    /// See QueryIntValue
    XMLError QueryBoolValue( bool* value ) const;
    /// See QueryIntValue
    XMLError QueryDoubleValue( double* value ) const;
    /// See QueryIntValue
    XMLError QueryFloatValue( float* value ) const;

    /// Set the attribute to a string value.
    void SetAttribute( const char* value );
    /// Set the attribute to value.
    void SetAttribute( int value );
    /// Set the attribute to value.
    void SetAttribute( unsigned value );
    /// Set the attribute to value.
    void SetAttribute( bool value );
    /// Set the attribute to value.
    void SetAttribute( double value );
    /// Set the attribute to value.
    void SetAttribute( float value );

private:
    enum { BUF_SIZE = 200 };

    XMLAttribute() : _next( 0 ), _memPool( 0 ) {}
    virtual ~XMLAttribute() {}

    XMLAttribute( const XMLAttribute& );    // not supported
    void operator=( const XMLAttribute& );  // not supported
    void SetName( const char* name );

    char* ParseDeep( char* p, bool processEntities );

    mutable StrPair _name;
    mutable StrPair _value;
    XMLAttribute*   _next;
    MemPool*        _memPool;
};

/** The element is a container class. It has a value, the element name,
    and can contain other elements, text, comments, and unknowns.
    Elements also contain an arbitrary number of attributes.
*/
class XMLElement : public XMLNode
{
    friend class XMLBase;
    friend class XMLDocument;
public:
    /// Get the name of an element (which is the Value() of the node.)
    const char* Name() const        {
        return Value();
    }
    /// Set the name of the element.
    void SetName( const char* str, bool staticMem=false )   {
        SetValue( str, staticMem );
    }

    virtual XMLElement* ToElement()             {
        return this;
    }
    virtual const XMLElement* ToElement() const {
        return this;
    }
    virtual bool Accept( XMLVisitor* visitor ) const;

    /** Given an attribute name, Attribute() returns the value
        for the attribute of that name, or null if none
        exists. For example:

        @verbatim
        const char* value = ele->Attribute( "foo" );
        @endverbatim

        The 'value' parameter is normally null. However, if specified,
        the attribute will only be returned if the 'name' and 'value'
        match. This allow you to write code:

        @verbatim
        if ( ele->Attribute( "foo", "bar" ) ) callFooIsBar();
        @endverbatim

        rather than:
        @verbatim
        if ( ele->Attribute( "foo" ) ) {
            if ( strcmp( ele->Attribute( "foo" ), "bar" ) == 0 ) callFooIsBar();
        }
        @endverbatim
    */
    const char* Attribute( const char* name, const char* value=0 ) const;

    /** Given an attribute name, IntAttribute() returns the value
        of the attribute interpreted as an integer. 0 will be
        returned if there is an error. For a method with error
        checking, see QueryIntAttribute()
    */
    int      IntAttribute( const char* name ) const     {
        int i=0;
        QueryIntAttribute( name, &i );
        return i;
    }
    /// See IntAttribute()
    unsigned UnsignedAttribute( const char* name ) const {
        unsigned i=0;
        QueryUnsignedAttribute( name, &i );
        return i;
    }
    /// See IntAttribute()
    bool     BoolAttribute( const char* name ) const    {
        bool b=false;
        QueryBoolAttribute( name, &b );
        return b;
    }
    /// See IntAttribute()
    double   DoubleAttribute( const char* name ) const  {
        double d=0;
        QueryDoubleAttribute( name, &d );
        return d;
    }
    /// See IntAttribute()
    float    FloatAttribute( const char* name ) const   {
        float f=0;
        QueryFloatAttribute( name, &f );
        return f;
    }

    /** Given an attribute name, QueryIntAttribute() returns
        XML_NO_ERROR, XML_WRONG_ATTRIBUTE_TYPE if the conversion
        can't be performed, or XML_NO_ATTRIBUTE if the attribute
        doesn't exist. If successful, the result of the conversion
        will be written to 'value'. If not successful, nothing will
        be written to 'value'. This allows you to provide default
        value:

        @verbatim
        int value = 10;
        QueryIntAttribute( "foo", &value );
        // if "foo" isn't found, value will still be 10
        @endverbatim
    */
    XMLError QueryIntAttribute( const char* name, int* value ) const {
        const XMLAttribute* a = FindAttribute( name );
        if ( !a ) {
            return XML_NO_ATTRIBUTE;
        }
        return a->QueryIntValue( value );
    }
    /// See QueryIntAttribute()
    XMLError QueryUnsignedAttribute( const char* name,
        unsigned int* value ) const  {
        const XMLAttribute* a = FindAttribute( name );
        if ( !a ) {
            return XML_NO_ATTRIBUTE;
        }
        return a->QueryUnsignedValue( value );
    }
    /// See QueryIntAttribute()
    XMLError QueryBoolAttribute( const char* name, bool* value ) const {
        const XMLAttribute* a = FindAttribute( name );
        if ( !a ) {
            return XML_NO_ATTRIBUTE;
        }
        return a->QueryBoolValue( value );
    }
    /// See QueryIntAttribute()
    XMLError QueryDoubleAttribute( const char* name, double* value ) const {
        const XMLAttribute* a = FindAttribute( name );
        if ( !a ) {
            return XML_NO_ATTRIBUTE;
        }
        return a->QueryDoubleValue( value );
    }
    /// See QueryIntAttribute()
    XMLError QueryFloatAttribute( const char* name, float* value ) const {
        const XMLAttribute* a = FindAttribute( name );
        if ( !a ) {
            return XML_NO_ATTRIBUTE;
        }
        return a->QueryFloatValue( value );
    }

    /** Given an attribute name, QueryAttribute() returns
        XML_NO_ERROR, XML_WRONG_ATTRIBUTE_TYPE if the conversion
        can't be performed, or XML_NO_ATTRIBUTE if the attribute
        doesn't exist. It is overloaded for the primitive types,
        and is a generally more convenient replacement of
        QueryIntAttribute() and related functions.

        If successful, the result of the conversion
        will be written to 'value'. If not successful, nothing will
        be written to 'value'. This allows you to provide default
        value:

        @verbatim
        int value = 10;
        // if "foo" isn't found, value will still be 10
        QueryAttribute( "foo", &value );
        @endverbatim
    */
    int QueryAttribute( const char* name, int* value ) const {
        return QueryIntAttribute( name, value );
    }

    int QueryAttribute( const char* name, unsigned int* value ) const {
        return QueryUnsignedAttribute( name, value );
    }

    int QueryAttribute( const char* name, bool* value ) const {
        return QueryBoolAttribute( name, value );
    }

    int QueryAttribute( const char* name, double* value ) const {
        return QueryDoubleAttribute( name, value );
    }

    int QueryAttribute( const char* name, float* value ) const {
        return QueryFloatAttribute( name, value );
    }

    /// Sets the named attribute to value.
    void SetAttribute( const char* name, const char* value )    {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }
    /// Sets the named attribute to value.
    void SetAttribute( const char* name, int value )            {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }
    /// Sets the named attribute to value.
    void SetAttribute( const char* name, unsigned value )       {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }
    /// Sets the named attribute to value.
    void SetAttribute( const char* name, bool value )           {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }
    /// Sets the named attribute to value.
    void SetAttribute( const char* name, double value )     {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }
    /// Sets the named attribute to value.
    void SetAttribute( const char* name, float value )      {
        XMLAttribute* a = FindOrCreateAttribute( name );
        a->SetAttribute( value );
    }

    /**
        Delete an attribute.
    */
    void DeleteAttribute( const char* name );

    /// Return the first attribute in the list.
    const XMLAttribute* FirstAttribute() const {
        return _rootAttribute;
    }
    /// Query a specific attribute in the list.
    const XMLAttribute* FindAttribute( const char* name ) const;

    /** Convenience function for easy access to the text inside an element.
        Although easy
        and concise, GetText() is limited compared to getting the XMLText child
        and accessing it directly.

        If the first child of 'this' is a XMLText, the GetText()
        returns the character string of the Text node, else null is returned.

        This is a convenient method for getting the text of simple contained
        text:
        @verbatim
        <foo>This is text</foo>
            const char* str = fooElement->GetText();
        @endverbatim

        'str' will be a pointer to "This is text".

        Note that this function can be misleading. If the element foo was
        created from
        this XML:
        @verbatim
            <foo><b>This is text</b></foo>
        @endverbatim

        then the value of str would be null. The first child node isn't a text
        node, it is
        another element. From this XML:
        @verbatim
            <foo>This is <b>text</b></foo>
        @endverbatim
        GetText() will return "This is ".
    */
    const char* GetText() const;

    /** Convenience function for easy access to the text inside an element.
        Although easy
        and concise, SetText() is limited compared to creating an XMLText child
        and mutating it directly.

        If the first child of 'this' is a XMLText, SetText() sets its value to
        the given string, otherwise it will create a first child that is an
        XMLText.

        This is a convenient method for setting the text of simple contained
        text:
        @verbatim
        <foo>This is text</foo>
            fooElement->SetText( "Hullaballoo!" );
        <foo>Hullaballoo!</foo>
        @endverbatim

        Note that this function can be misleading. If the element foo was
        created from
        this XML:
        @verbatim
            <foo><b>This is text</b></foo>
        @endverbatim

        then it will not change "This is text", but rather prefix it with a text
        element:
        @verbatim
            <foo>Hullaballoo!<b>This is text</b></foo>
        @endverbatim

        For this XML:
        @verbatim
            <foo />
        @endverbatim
        SetText() will generate
        @verbatim
            <foo>Hullaballoo!</foo>
        @endverbatim
    */
    void SetText( const char* inText );
    /// Convenience method for setting text inside and element. See SetText().
    void SetText( int value );
    /// Convenience method for setting text inside and element. See SetText().
    void SetText( unsigned value );
    /// Convenience method for setting text inside and element. See SetText().
    void SetText( bool value );
    /// Convenience method for setting text inside and element. See SetText().
    void SetText( double value );
    /// Convenience method for setting text inside and element. See SetText().
    void SetText( float value );

    /**
        Convenience method to query the value of a child text node.
        This is probably best
        shown by example. Given you have a document is this form:
        @verbatim
            <point>
                <x>1</x>
                <y>1.4</y>
            </point>
        @endverbatim

        The QueryIntText() and similar functions provide a safe and easier way
        to get to the
        "value" of x and y.

        @verbatim
            int x = 0;
            float y = 0;    // types of x and y are contrived for example
            const XMLElement* xElement = pointElement->FirstChildElement( "x" );
            const XMLElement* yElement = pointElement->FirstChildElement( "y" );
            xElement->QueryIntText( &x );
            yElement->QueryFloatText( &y );
        @endverbatim

        @returns XML_SUCCESS (0) on success, XML_CAN_NOT_CONVERT_TEXT if the
        text cannot be converted
                 to the requested type, and XML_NO_TEXT_NODE if there is no
                 child text to query.

    */
    XMLError QueryIntText( int* ival ) const;
    /// See QueryIntText()
    XMLError QueryUnsignedText( unsigned* uval ) const;
    /// See QueryIntText()
    XMLError QueryBoolText( bool* bval ) const;
    /// See QueryIntText()
    XMLError QueryDoubleText( double* dval ) const;
    /// See QueryIntText()
    XMLError QueryFloatText( float* fval ) const;

    // internal:
    enum {
        OPEN,       // <foo>
        CLOSED,     // <foo/>
        CLOSING     // </foo>
    };
    int ClosingType() const {
        return _closingType;
    }
    char* ParseDeep( char* p, StrPair* endTag );
    virtual XMLNode* ShallowClone( XMLDocument* document ) const;
    virtual bool ShallowEqual( const XMLNode* compare ) const;

private:
    XMLElement( XMLDocument* doc );
    virtual ~XMLElement();
    XMLElement( const XMLElement& );    // not supported
    void operator=( const XMLElement& );    // not supported

    XMLAttribute* FindAttribute( const char* name );
    XMLAttribute* FindOrCreateAttribute( const char* name );
    //void LinkAttribute( XMLAttribute* attrib );
    char* ParseAttributes( char* p );

    enum { BUF_SIZE = 200 };
    int _closingType;
    // The attribute list is ordered; there is no 'lastAttribute'
    // because the list needs to be scanned for dupes before adding
    // a new attribute.
    XMLAttribute* _rootAttribute;
};

enum Whitespace {
    PRESERVE_WHITESPACE,
    COLLAPSE_WHITESPACE
};

/** A Document binds together all the functionality.
    It can be saved, loaded, and printed to the screen.
    All Nodes are connected and allocated to a Document.
    If the Document is deleted, all its Nodes are also deleted.
*/
class XMLDocument : public XMLNode
{
    friend class XMLElement;
public:
    /// constructor
    XMLDocument( bool processEntities = true,
        Whitespace = PRESERVE_WHITESPACE );
    ~XMLDocument();

    virtual XMLDocument* ToDocument()               {
        return this;
    }
    virtual const XMLDocument* ToDocument() const   {
        return this;
    }

    /**
        Parse an XML file from a character string.
        Returns XML_NO_ERROR (0) on success, or
        an errorID.

        You may optionally pass in the 'nBytes', which is
        the number of bytes which will be parsed. If not
        specified, TinyXML-2 will assume 'xml' points to a
        null terminated string.
    */
    XMLError Parse( const char* xml, unsigned int nBytes=(unsigned int)(-1) );

    bool ProcessEntities() const        {
        return _processEntities;
    }
    Whitespace WhitespaceMode() const   {
        return _whitespace;
    }

    /**
        Returns true if this document has a leading Byte Order Mark of UTF8.
    */
    bool HasBOM() const {
        return _writeBOM;
    }
    /** Sets whether to write the BOM when writing the file.
    */
    void SetBOM( bool useBOM ) {
        _writeBOM = useBOM;
    }

    /** Return the root element of DOM. Equivalent to FirstChildElement().
        To get the first node, use FirstChild().
    */
    XMLElement* RootElement()               {
        return FirstChildElement();
    }
    const XMLElement* RootElement() const   {
        return FirstChildElement();
    }

    /** Print the Document. If the Printer is not provided, it will
        print to stdout. If you provide Printer, this can print to a file:
        @verbatim
        XMLPrinter printer( fp );
        doc.Print( &printer );
        @endverbatim

        Or you can use a printer to print to memory:
        @verbatim
        XMLPrinter printer;
        doc.Print( &printer );
        // printer.CStr() has a const char* to the XML
        @endverbatim
    */
    void Print( XMLPrinter* streamer=0 ) const;
    virtual bool Accept( XMLVisitor* visitor ) const;

    /**
        Create a new Element associated with
        this Document. The memory for the Element
        is managed by the Document.
    */
    XMLElement* NewElement( const char* name );
    /**
        Create a new Comment associated with
        this Document. The memory for the Comment
        is managed by the Document.
    */
    XMLComment* NewComment( const char* comment );
    /**
        Create a new Text associated with
        this Document. The memory for the Text
        is managed by the Document.
    */
    XMLText* NewText( const char* text );
    /**
        Create a new Declaration associated with
        this Document. The memory for the object
        is managed by the Document.

        If the 'text' param is null, the standard
        declaration is used.:
        @verbatim
            <?xml version="1.0" encoding="UTF-8"?>
        @endverbatim
    */
    XMLDeclaration* NewDeclaration( const char* text=0 );
    /**
        Create a new Unknown associated with
        this Document. The memory for the object
        is managed by the Document.
    */
    XMLUnknown* NewUnknown( const char* text );

    /**
        Delete a node associated with this document.
        It will be unlinked from the DOM.
    */
    void DeleteNode( XMLNode* node )    {
        node->_parent->DeleteChild( node );
    }

    void SetError( XMLError error, const char* str1, const char* str2 );

    /// Return true if there was an error parsing the document.
    bool Error() const {
        return _errorID != XML_NO_ERROR;
    }
    /// Return the errorID.
    XMLError  ErrorID() const {
        return _errorID;
    }
    /// Return a possibly helpful diagnostic location or string.
    const char* GetErrorStr1() const {
        return _errorStr1;
    }
    /// Return a possibly helpful secondary diagnostic location or string.
    const char* GetErrorStr2() const {
        return _errorStr2;
    }
    /// If there is an error, print it to stdout.
    void PrintError() const;

    /// Clear the document, resetting it to the initial state.
    void Clear();

    // internal
    char* Identify( char* p, XMLNode** node );

    virtual XMLNode* ShallowClone( XMLDocument* /*document*/ ) const    {
        return 0;
    }
    virtual bool ShallowEqual( const XMLNode* /*compare*/ ) const   {
        return false;
    }

private:
    XMLDocument( const XMLDocument& );  // not supported
    void operator=( const XMLDocument& );   // not supported

    bool        _writeBOM;
    bool        _processEntities;
    XMLError    _errorID;
    Whitespace  _whitespace;
    const char* _errorStr1;
    const char* _errorStr2;
    char*       _charBuffer;

    MemPoolT< sizeof(XMLElement) >   _elementPool;
    MemPoolT< sizeof(XMLAttribute) > _attributePool;
    MemPoolT< sizeof(XMLText) >      _textPool;
    MemPoolT< sizeof(XMLComment) >   _commentPool;
};

/**
    A XMLHandle is a class that wraps a node pointer with null checks; this is
    an incredibly useful thing. Note that XMLHandle is not part of the TinyXML-2
    DOM structure. It is a separate utility class.

    Take an example:
    @verbatim
    <Document>
        <Element attributeA = "valueA">
            <Child attributeB = "value1" />
            <Child attributeB = "value2" />
        </Element>
    </Document>
    @endverbatim

    Assuming you want the value of "attributeB" in the 2nd "Child" element,
    it's very
    easy to write a *lot* of code that looks like:

    @verbatim
    XMLElement* root = document.FirstChildElement( "Document" );
    if ( root )
    {
        XMLElement* element = root->FirstChildElement( "Element" );
        if ( element )
        {
            XMLElement* child = element->FirstChildElement( "Child" );
            if ( child )
            {
                XMLElement* child2 = child->NextSiblingElement( "Child" );
                if ( child2 )
                {
                    // Finally do something useful.
    @endverbatim

    And that doesn't even cover "else" cases. XMLHandle addresses the verbosity
    of such code. A XMLHandle checks for null pointers so it is perfectly safe
    and correct to use:

    @verbatim
    XMLHandle docHandle( &document );
    XMLElement* child2 = docHandle.FirstChild( "Document" ).FirstChild(
        "Element" ).FirstChild().NextSibling().ToElement();
    if ( child2 )
    {
        // do something useful
    @endverbatim

    Which is MUCH more concise and useful.

    It is also safe to copy handles - internally they are nothing more than
    node pointers.
    @verbatim
    XMLHandle handleCopy = handle;
    @endverbatim

    See also XMLConstHandle, which is the same as XMLHandle, but operates on
    const objects.
*/
class XMLHandle
{
public:
    /** Create a handle from any node (at any depth of the tree.) This can be a
    null pointer.*/
    XMLHandle( XMLNode* node )                                              {
        _node = node;
    }
    /// Create a handle from a node.
    XMLHandle( XMLNode& node )                                              {
        _node = &node;
    }
    /// Copy constructor
    XMLHandle( const XMLHandle& ref )                                       {
        _node = ref._node;
    }
    /// Assignment
    XMLHandle& operator=( const XMLHandle& ref )                            {
        _node = ref._node;
        return *this;
    }

    /// Get the first child of this handle.
    XMLHandle FirstChild()                                                  {
        return XMLHandle( _node ? _node->FirstChild() : 0 );
    }
    /// Get the first child element of this handle.
    XMLHandle FirstChildElement( const char* value=0 )                      {
        return XMLHandle( _node ? _node->FirstChildElement( value ) : 0 );
    }
    /// Get the last child of this handle.
    XMLHandle LastChild()                                                   {
        return XMLHandle( _node ? _node->LastChild() : 0 );
    }
    /// Get the last child element of this handle.
    XMLHandle LastChildElement( const char* _value=0 )                      {
        return XMLHandle( _node ? _node->LastChildElement( _value ) : 0 );
    }
    /// Get the previous sibling of this handle.
    XMLHandle PreviousSibling()                                             {
        return XMLHandle( _node ? _node->PreviousSibling() : 0 );
    }
    /// Get the previous sibling element of this handle.
    XMLHandle PreviousSiblingElement( const char* _value=0 )                {
        return XMLHandle( _node ? _node->PreviousSiblingElement( _value ) : 0 );
    }
    /// Get the next sibling of this handle.
    XMLHandle NextSibling()                                                 {
        return XMLHandle( _node ? _node->NextSibling() : 0 );
    }
    /// Get the next sibling element of this handle.
    XMLHandle NextSiblingElement( const char* _value=0 )                    {
        return XMLHandle( _node ? _node->NextSiblingElement( _value ) : 0 );
    }

    /// Safe cast to XMLNode. This can return null.
    XMLNode* ToNode()                           {
        return _node;
    }
    /// Safe cast to XMLElement. This can return null.
    XMLElement* ToElement()                     {
        return ( ( _node && _node->ToElement() ) ? _node->ToElement() : 0 );
    }
    /// Safe cast to XMLText. This can return null.
    XMLText* ToText()                           {
        return ( ( _node && _node->ToText() ) ? _node->ToText() : 0 );
    }
    /// Safe cast to XMLUnknown. This can return null.
    XMLUnknown* ToUnknown()                     {
        return ( ( _node && _node->ToUnknown() ) ? _node->ToUnknown() : 0 );
    }
    /// Safe cast to XMLDeclaration. This can return null.
    XMLDeclaration* ToDeclaration()             {
        return ( ( _node && _node->ToDeclaration() ) ?
            _node->ToDeclaration() : 0 );
    }

private:
    XMLNode* _node;
};

/**
    A variant of the XMLHandle class for working with const XMLNodes and
    Documents. It is the
    same in all regards, except for the 'const' qualifiers. See XMLHandle for
    API.
*/
class XMLConstHandle
{
public:
    XMLConstHandle( const XMLNode* node )                             {
        _node = node;
    }
    XMLConstHandle( const XMLNode& node )                             {
        _node = &node;
    }
    XMLConstHandle( const XMLConstHandle& ref )                       {
        _node = ref._node;
    }

    XMLConstHandle& operator=( const XMLConstHandle& ref )            {
        _node = ref._node;
        return *this;
    }

    const XMLConstHandle FirstChild() const                           {
        return XMLConstHandle( _node ? _node->FirstChild() : 0 );
    }
    const XMLConstHandle FirstChildElement( const char* value=0 ) const      {
        return XMLConstHandle( _node ? _node->FirstChildElement( value ) : 0 );
    }
    const XMLConstHandle LastChild()    const                  {
        return XMLConstHandle( _node ? _node->LastChild() : 0 );
    }
    const XMLConstHandle LastChildElement( const char* _value=0 ) const      {
        return XMLConstHandle( _node ? _node->LastChildElement( _value ) : 0 );
    }
    const XMLConstHandle PreviousSibling() const                             {
        return XMLConstHandle( _node ? _node->PreviousSibling() : 0 );
    }
    const XMLConstHandle PreviousSiblingElement( const char* _value=0 ) const  {
        return XMLConstHandle( _node ? _node->PreviousSiblingElement(
            _value ) : 0 );
    }
    const XMLConstHandle NextSibling() const                                 {
        return XMLConstHandle( _node ? _node->NextSibling() : 0 );
    }
    const XMLConstHandle NextSiblingElement( const char* _value=0 ) const    {
        return XMLConstHandle( _node ? _node->NextSiblingElement(
            _value ) : 0 );
    }

    const XMLNode* ToNode() const               {
        return _node;
    }
    const XMLElement* ToElement() const         {
        return ( ( _node && _node->ToElement() ) ? _node->ToElement() : 0 );
    }
    const XMLText* ToText() const               {
        return ( ( _node && _node->ToText() ) ? _node->ToText() : 0 );
    }
    const XMLUnknown* ToUnknown() const         {
        return ( ( _node && _node->ToUnknown() ) ? _node->ToUnknown() : 0 );
    }
    const XMLDeclaration* ToDeclaration() const {
        return ( ( _node && _node->ToDeclaration() ) ?
            _node->ToDeclaration() : 0 );
    }

private:
    const XMLNode* _node;
};

/**
    Printing functionality. The XMLPrinter gives you more
    options than the XMLDocument::Print() method.

    It can:
    -# Print to memory.
    -# Print to a file you provide.
    -# Print XML without a XMLDocument.

    Print to Memory

    @verbatim
    XMLPrinter printer;
    doc.Print( &printer );
    SomeFunction( printer.CStr() );
    @endverbatim

    Print to a File

    You provide the file pointer.
    @verbatim
    XMLPrinter printer( fp );
    doc.Print( &printer );
    @endverbatim

    Print without a XMLDocument

    When loading, an XML parser is very useful. However, sometimes
    when saving, it just gets in the way. The code is often set up
    for streaming, and constructing the DOM is just overhead.

    The Printer supports the streaming case. The following code
    prints out a trivially simple XML file without ever creating
    an XML document.

    @verbatim
    XMLPrinter printer( fp );
    printer.OpenElement( "foo" );
    printer.PushAttribute( "foo", "bar" );
    printer.CloseElement();
    @endverbatim
*/
class XMLPrinter : public XMLVisitor
{
public:
    /** Construct the printer. If the FILE* is specified,
        this will print to the FILE. Else it will print
        to memory, and the result is available in CStr().
        If 'compact' is set to true, then output is created
        with only required whitespace and newlines.
    */
    XMLPrinter( bool compact = false, int depth = 0 );
    virtual ~XMLPrinter()   {}

    /** If streaming, write the BOM and declaration. */
    void PushHeader( bool writeBOM, bool writeDeclaration );
    /** If streaming, start writing an element.
        The element must be closed with CloseElement()
    */
    void OpenElement( const char* name, bool compactMode=false );
    /// If streaming, add an attribute to an open element.
    void PushAttribute( const char* name, const char* value );
    void PushAttribute( const char* name, int value );
    void PushAttribute( const char* name, unsigned value );
    void PushAttribute( const char* name, bool value );
    void PushAttribute( const char* name, double value );
    /// If streaming, close the Element.
    virtual void CloseElement( bool compactMode=false );

    /// Add a text node.
    void PushText( const char* text, bool cdata=false );
    /// Add a text node from an integer.
    void PushText( int value );
    /// Add a text node from an unsigned.
    void PushText( unsigned value );
    /// Add a text node from a bool.
    void PushText( bool value );
    /// Add a text node from a float.
    void PushText( float value );
    /// Add a text node from a double.
    void PushText( double value );

    /// Add a comment
    void PushComment( const char* comment );

    void PushDeclaration( const char* value );
    void PushUnknown( const char* value );

    virtual bool VisitEnter( const XMLDocument& /*doc*/ );
    virtual bool VisitExit( const XMLDocument& /*doc*/ )            {
        return true;
    }

    virtual bool VisitEnter( const XMLElement& element,
        const XMLAttribute* attribute );
    virtual bool VisitExit( const XMLElement& element );

    virtual bool Visit( const XMLText& text );
    virtual bool Visit( const XMLComment& comment );
    virtual bool Visit( const XMLDeclaration& declaration );
    virtual bool Visit( const XMLUnknown& unknown );

    /**
        If in print to memory mode, return a pointer to
        the XML file in memory.
    */
    const char* CStr() const {
        return _buffer.Mem();
    }
    /**
        If in print to memory mode, return the size
        of the XML file in memory. (Note the size returned
        includes the terminating null.)
    */
    int CStrSize() const {
        return _buffer.Size();
    }
    /**
        If in print to memory mode, reset the buffer to the
        beginning.
    */
    void ClearBuffer() {
        _buffer.Clear();
        _buffer.Push(0);
    }

protected:
    virtual bool CompactMode( const XMLElement& )   { return _compactMode; }

    /** Prints out the space before an element. You may override to change
        the space and tabs used. A PrintSpace() override should call Print().
    */
    virtual void PrintSpace( int depth );
    void Print( const char* format, ... );

    void SealElement();
    bool _elementJustOpened;
    DynArray< const char*, 10 > _stack;

private:
    void PrintString( const char*, bool restrictedEntitySet );

    bool _firstElement;
    int _depth;
    int _textDepth;
    bool _processEntities;
    bool _compactMode;

    enum {
        ENTITY_RANGE = 64,
        BUF_SIZE = 200
    };
    bool _entityFlag[ENTITY_RANGE];
    bool _restrictedEntityFlag[ENTITY_RANGE];

    DynArray< char, 20 > _buffer;
};

}   // tinyxml2
}}

////////////////////////////////////////////////////////////////////////////////

#ifdef PRIM_COMPILE_INLINE

namespace PRIM_NAMESPACE { namespace meta {

static const char LINE_FEED             = (char)0x0a;
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN       = (char)0x0d;
static const char CR = CARRIAGE_RETURN;
static const char SINGLE_QUOTE          = '\'';
static const char DOUBLE_QUOTE          = '\"';

// Bunch of unicode info at:
//      http://www.unicode.org/faq/utf_bom.html
//  ef bb bf (Microsoft "lead bytes") - designates UTF-8

static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

#define PRIM_TINYXML_DELETE_NODE( node ) {           \
        if ( node ) {                       \
            MemPool* pool = node->_memPool; \
            node->~XMLNode();               \
            pool->Free( node );             \
        }                                   \
    }
#define PRIM_TINYXML_DELETE_ATTRIBUTE( attrib ) {        \
        if ( attrib ) {                         \
            MemPool* pool = attrib->_memPool;   \
            attrib->~XMLAttribute();            \
            pool->Free( attrib );               \
        }                                       \
    }

namespace tinyxml2
{

struct Entity {
    const char* pattern;
    int length;
    char value;
};

static const int NUM_ENTITIES = 5;
static const Entity entities[NUM_ENTITIES] = {
    { "quot", 4,    DOUBLE_QUOTE },
    { "amp", 3,     '&'  },
    { "apos", 4,    SINGLE_QUOTE },
    { "lt", 2,      '<'  },
    { "gt", 2,      '>'  }
};

StrPair::~StrPair()
{
    Reset();
}

void StrPair::Reset()
{
    if ( _flags & NEEDS_DELETE ) {
        delete [] _start;
    }
    _flags = 0;
    _start = 0;
    _end = 0;
}

void StrPair::SetStr( const char* str, int flags )
{
    Reset();
    unsigned int len = (unsigned int)strlen( str );
    _start = new char[ len+1 ];
    memcpy( _start, str, len+1 );
    _end = _start + len;
    _flags = flags | NEEDS_DELETE;
}

char* StrPair::ParseText( char* p, const char* endTag, int strFlags )
{
    char* start = p;    // fixme: hides a member
    char  endChar = *endTag;
    unsigned int length = (unsigned int)strlen( endTag );

    // Inner loop of text parsing.
    while ( *p ) {
        if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
            Set( start, p, strFlags );
            return p + length;
        }
        ++p;
    }
    return 0;
}

char* StrPair::ParseName( char* p )
{
    char* start = p;

    if ( !start || !(*start) ) {
        return 0;
    }

    while( *p && ( p == start ? XMLUtil::IsNameStartChar( *p ) :
        XMLUtil::IsNameChar( *p ) )) {
        ++p;
    }

    if ( p > start ) {
        Set( start, p, 0 );
        return p;
    }
    return 0;
}

void StrPair::CollapseWhitespace()
{
    // Trim leading space.
    _start = XMLUtil::SkipWhiteSpace( _start );

    if ( _start && *_start ) {
        char* p = _start;   // the read pointer
        char* q = _start;   // the write pointer

        while( *p ) {
            if ( XMLUtil::IsWhiteSpace( *p )) {
                p = XMLUtil::SkipWhiteSpace( p );
                if ( *p == 0 ) {
                    break;  // don't write to q; this trims the trailing space.
                }
                *q = ' ';
                ++q;
            }
            *q = *p;
            ++q;
            ++p;
        }
        *q = 0;
    }
}

const char* StrPair::GetStr()
{
    if ( _flags & NEEDS_FLUSH ) {
        *_end = 0;
        _flags ^= NEEDS_FLUSH;

        if ( _flags ) {
            char* p = _start;   // the read pointer
            char* q = _start;   // the write pointer

            while( p < _end ) {
                if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == CR ) {
                    // CR-LF pair becomes LF
                    // CR alone becomes LF
                    // LF-CR becomes LF
                    if ( *(p+1) == LF ) {
                        p += 2;
                    }
                    else {
                        ++p;
                    }
                    *q++ = LF;
                }
                else if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == LF ) {
                    if ( *(p+1) == CR ) {
                        p += 2;
                    }
                    else {
                        ++p;
                    }
                    *q++ = LF;
                }
                else if ( (_flags & NEEDS_ENTITY_PROCESSING) && *p == '&' ) {
                    // Entities handled by tinyXML2:
                    // - special entities in the entity table [in/out]
                    // - numeric character reference [in]
                    //   &#20013; or &#x4e2d;

                    if ( *(p+1) == '#' ) {
                        char buf[10] = { 0 };
                        int len;
                        p = const_cast<char*>( XMLUtil::GetCharacterRef(
                            p, buf, &len ) );
                        for( int i=0; i<len; ++i ) {
                            *q++ = buf[i];
                        }
                    }
                    else {
                        int i=0;
                        for(; i<NUM_ENTITIES; ++i ) {
                            if (    strncmp( p+1, entities[i].pattern,
                                entities[i].length ) == 0
                                    && *(p+entities[i].length+1) == ';' ) {
                                // Found an entity convert;
                                *q = entities[i].value;
                                ++q;
                                p += entities[i].length + 2;
                                break;
                            }
                        }
                        if ( i == NUM_ENTITIES ) {
                            // fixme: treat as error?
                            ++p;
                            ++q;
                        }
                    }
                }
                else {
                    *q = *p;
                    ++p;
                    ++q;
                }
            }
            *q = 0;
        }
        // The loop below has plenty going on, and this
        // is a less useful mode. Break it out.
        if ( _flags & COLLAPSE_WHITESPACE ) {
            CollapseWhitespace();
        }
        _flags = (_flags & NEEDS_DELETE);
    }
    return _start;
}

// --------- XMLUtil ----------- //

// Anything in the high order range of UTF-8 is assumed to not be
// whitespace. This isn't
// correct, but simple, and usually works.
const char* XMLUtil::SkipWhiteSpace( const char* p )  {
    while( !IsUTF8Continuation(*p) && isspace(
        *reinterpret_cast<const unsigned char*>(p) ) ) {
        ++p;
    }
    return p;
}

char* XMLUtil::SkipWhiteSpace( char* p )              {
    while( !IsUTF8Continuation(*p) && isspace(
        *reinterpret_cast<unsigned char*>(p) ) )     {
        ++p;
    }
    return p;
}

bool XMLUtil::IsWhiteSpace( char p )                  {
    return !IsUTF8Continuation(p) && isspace(
        static_cast<unsigned char>(p) );
}

bool XMLUtil::IsNameStartChar( unsigned char ch ) {
    return ( ( ch < 128 ) ? isalpha( ch ) : 1 )
           || ch == ':'
           || ch == '_';
}

bool XMLUtil::IsNameChar( unsigned char ch ) {
    return IsNameStartChar( ch )
           || isdigit( ch )
           || ch == '.'
           || ch == '-';
}

const char* XMLUtil::ReadBOM( const char* p, bool* bom )
{
    *bom = false;
    const unsigned char* pu = reinterpret_cast<const unsigned char*>(p);
    // Check for BOM:
    if (    *(pu+0) == TIXML_UTF_LEAD_0
            && *(pu+1) == TIXML_UTF_LEAD_1
            && *(pu+2) == TIXML_UTF_LEAD_2 ) {
        *bom = true;
        p += 3;
    }
    return p;
}

void XMLUtil::ConvertUTF32ToUTF8( unsigned long input, char* output,
    int* length )
{
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0,
        0xF8, 0xFC };

    if (input < 0x80) {
        *length = 1;
    }
    else if ( input < 0x800 ) {
        *length = 2;
    }
    else if ( input < 0x10000 ) {
        *length = 3;
    }
    else if ( input < 0x200000 ) {
        *length = 4;
    }
    else {
        *length = 0;    // This code won't covert this correctly anyway.
        return;
    }

    output += *length;

    // Scary scary fall throughs.
    switch (*length) {
        case 4:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 3:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 2:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 1:
            --output;
            *output = (char)(input | FIRST_BYTE_MARK[*length]);
        default:
            break;
    }
}

const char* XMLUtil::GetCharacterRef( const char* p, char* value, int* length )
{
    // Presume an entity, and pull it out.
    *length = 0;

    if ( *(p+1) == '#' && *(p+2) ) {
        unsigned long ucs = 0;
        ptrdiff_t delta = 0;
        unsigned mult = 1;

        if ( *(p+2) == 'x' ) {
            // Hexadecimal.
            if ( !*(p+3) ) {
                return 0;
            }

            const char* q = p+3;
            q = strchr( q, ';' );

            if ( !q || !*q ) {
                return 0;
            }

            delta = q-p;
            --q;

            while ( *q != 'x' ) {
                if ( *q >= '0' && *q <= '9' ) {
                    ucs += mult * (*q - '0');
                }
                else if ( *q >= 'a' && *q <= 'f' ) {
                    ucs += mult * (*q - 'a' + 10);
                }
                else if ( *q >= 'A' && *q <= 'F' ) {
                    ucs += mult * (*q - 'A' + 10 );
                }
                else {
                    return 0;
                }
                mult *= 16;
                --q;
            }
        }
        else {
            // Decimal.
            if ( !*(p+2) ) {
                return 0;
            }

            const char* q = p+2;
            q = strchr( q, ';' );

            if ( !q || !*q ) {
                return 0;
            }

            delta = q-p;
            --q;

            while ( *q != '#' ) {
                if ( *q >= '0' && *q <= '9' ) {
                    ucs += mult * (*q - '0');
                }
                else {
                    return 0;
                }
                mult *= 10;
                --q;
            }
        }
        // convert the UCS to UTF-8
        ConvertUTF32ToUTF8( ucs, value, length );
        return p + delta + 1;
    }
    return p+1;
}

void XMLUtil::ToStr( int v, char* buffer, int bufferSize )
{
    PRIM_TIXML_SNPRINTF( buffer, bufferSize, "%d", v );
}

void XMLUtil::ToStr( unsigned v, char* buffer, int bufferSize )
{
    PRIM_TIXML_SNPRINTF( buffer, bufferSize, "%u", v );
}

void XMLUtil::ToStr( bool v, char* buffer, int bufferSize )
{
    PRIM_TIXML_SNPRINTF( buffer, bufferSize, "%d", v ? 1 : 0 );
}

/*
    ToStr() of a number is a very tricky topic.
    https://github.com/leethomason/tinyxml2/issues/106
*/
void XMLUtil::ToStr( float v, char* buffer, int bufferSize )
{
    PRIM_TIXML_SNPRINTF( buffer, bufferSize, "%.8g", v );
}

void XMLUtil::ToStr( double v, char* buffer, int bufferSize )
{
    PRIM_TIXML_SNPRINTF( buffer, bufferSize, "%.17g", v );
}

bool XMLUtil::ToInt( const char* str, int* value )
{
    if ( PRIM_TIXML_SSCANF( str, "%d", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToUnsigned( const char* str, unsigned *value )
{
    if ( PRIM_TIXML_SSCANF( str, "%u", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToBool( const char* str, bool* value )
{
    int ival = 0;
    if ( ToInt( str, &ival )) {
        *value = (ival==0) ? false : true;
        return true;
    }
    if ( StringEqual( str, "true" ) ) {
        *value = true;
        return true;
    }
    else if ( StringEqual( str, "false" ) ) {
        *value = false;
        return true;
    }
    return false;
}

bool XMLUtil::ToFloat( const char* str, float* value )
{
    if ( PRIM_TIXML_SSCANF( str, "%f", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToDouble( const char* str, double* value )
{
    if ( PRIM_TIXML_SSCANF( str, "%lf", value ) == 1 ) {
        return true;
    }
    return false;
}

char* XMLDocument::Identify( char* p, XMLNode** node )
{
    XMLNode* returnNode = 0;
    char* start = p;
    p = XMLUtil::SkipWhiteSpace( p );
    if( !p || !*p ) {
        return p;
    }

    // What is this thing?
    // These strings define the matching patters:
    static const char* xmlHeader        = { "<?" };
    static const char* commentHeader    = { "<!--" };
    static const char* dtdHeader        = { "<!" };
    static const char* cdataHeader      = { "<![CDATA[" };
    static const char* elementHeader    = { "<" };

    static const int xmlHeaderLen       = 2;
    static const int commentHeaderLen   = 4;
    static const int dtdHeaderLen       = 2;
    static const int cdataHeaderLen     = 9;
    static const int elementHeaderLen   = 1;

    if ( XMLUtil::StringEqual( p, xmlHeader, xmlHeaderLen ) ) {
        returnNode = new (_commentPool.Alloc()) XMLDeclaration( this );
        returnNode->_memPool = &_commentPool;
        p += xmlHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, commentHeader, commentHeaderLen ) ) {
        returnNode = new (_commentPool.Alloc()) XMLComment( this );
        returnNode->_memPool = &_commentPool;
        p += commentHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, cdataHeader, cdataHeaderLen ) ) {
        XMLText* text = new (_textPool.Alloc()) XMLText( this );
        returnNode = text;
        returnNode->_memPool = &_textPool;
        p += cdataHeaderLen;
        text->SetCData( true );
    }
    else if ( XMLUtil::StringEqual( p, dtdHeader, dtdHeaderLen ) ) {
        returnNode = new (_commentPool.Alloc()) XMLUnknown( this );
        returnNode->_memPool = &_commentPool;
        p += dtdHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, elementHeader, elementHeaderLen ) ) {
        returnNode = new (_elementPool.Alloc()) XMLElement( this );
        returnNode->_memPool = &_elementPool;
        p += elementHeaderLen;
    }
    else {
        returnNode = new (_textPool.Alloc()) XMLText( this );
        returnNode->_memPool = &_textPool;
        p = start;  // Back it up, all the text counts.
    }

    *node = returnNode;
    return p;
}

bool XMLDocument::Accept( XMLVisitor* visitor ) const
{
    if ( visitor->VisitEnter( *this ) ) {
        for ( const XMLNode* node=FirstChild(); node;
            node=node->NextSibling() ) {
            if ( !node->Accept( visitor ) ) {
                break;
            }
        }
    }
    return visitor->VisitExit( *this );
}

// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
    _document( doc ),
    _parent( 0 ),
    _firstChild( 0 ), _lastChild( 0 ),
    _prev( 0 ), _next( 0 ),
    _memPool( 0 )
{
}

XMLNode::~XMLNode()
{
    DeleteChildren();
    if ( _parent ) {
        _parent->Unlink( this );
    }
}

const char* XMLNode::Value() const
{
    return _value.GetStr();
}

void XMLNode::SetValue( const char* str, bool staticMem )
{
    if ( staticMem ) {
        _value.SetInternedStr( str );
    }
    else {
        _value.SetStr( str );
    }
}

void XMLNode::DeleteChildren()
{
    while( _firstChild ) {
        XMLNode* node = _firstChild;
        Unlink( node );

        PRIM_TINYXML_DELETE_NODE( node );
    }
    _firstChild = _lastChild = 0;
}

void XMLNode::Unlink( XMLNode* child )
{
    if ( child == _firstChild ) {
        _firstChild = _firstChild->_next;
    }
    if ( child == _lastChild ) {
        _lastChild = _lastChild->_prev;
    }

    if ( child->_prev ) {
        child->_prev->_next = child->_next;
    }
    if ( child->_next ) {
        child->_next->_prev = child->_prev;
    }
    child->_parent = 0;
}

void XMLNode::DeleteChild( XMLNode* node )
{
    PRIM_TINYXML_DELETE_NODE( node );
}

XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
    if (addThis->_document != _document)
        return 0;

    if (addThis->_parent)
        addThis->_parent->Unlink( addThis );
    else
       addThis->_memPool->SetTracked();

    if ( _lastChild ) {
        _lastChild->_next = addThis;
        addThis->_prev = _lastChild;
        _lastChild = addThis;

        addThis->_next = 0;
    }
    else {
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
    return addThis;
}

XMLNode* XMLNode::InsertFirstChild( XMLNode* addThis )
{
    if (addThis->_document != _document)
        return 0;

    if (addThis->_parent)
        addThis->_parent->Unlink( addThis );
    else
       addThis->_memPool->SetTracked();

    if ( _firstChild ) {
        _firstChild->_prev = addThis;
        addThis->_next = _firstChild;
        _firstChild = addThis;

        addThis->_prev = 0;
    }
    else {
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
     return addThis;
}

XMLNode* XMLNode::InsertAfterChild( XMLNode* afterThis, XMLNode* addThis )
{
    if (addThis->_document != _document)
        return 0;

    if ( afterThis->_parent != this ) {
        return 0;
    }

    if ( afterThis->_next == 0 ) {
        // The last node or the only node.
        return InsertEndChild( addThis );
    }
    if (addThis->_parent)
        addThis->_parent->Unlink( addThis );
    else
       addThis->_memPool->SetTracked();
    addThis->_prev = afterThis;
    addThis->_next = afterThis->_next;
    afterThis->_next->_prev = addThis;
    afterThis->_next = addThis;
    addThis->_parent = this;
    return addThis;
}

const XMLElement* XMLNode::FirstChildElement( const char* value ) const
{
    for( XMLNode* node=_firstChild; node; node=node->_next ) {
        XMLElement* element = node->ToElement();
        if ( element ) {
            if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
                return element;
            }
        }
    }
    return 0;
}

const XMLElement* XMLNode::LastChildElement( const char* value ) const
{
    for( XMLNode* node=_lastChild; node; node=node->_prev ) {
        XMLElement* element = node->ToElement();
        if ( element ) {
            if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
                return element;
            }
        }
    }
    return 0;
}

const XMLElement* XMLNode::NextSiblingElement( const char* value ) const
{
    for( XMLNode* element=this->_next; element; element = element->_next ) {
        if (    element->ToElement()
                && (!value || XMLUtil::StringEqual( value, element->Value() )))
        {
            return element->ToElement();
        }
    }
    return 0;
}

const XMLElement* XMLNode::PreviousSiblingElement( const char* value ) const
{
    for( XMLNode* element=_prev; element; element = element->_prev ) {
        if (    element->ToElement()
                && (!value || XMLUtil::StringEqual( value, element->Value() )))
        {
            return element->ToElement();
        }
    }
    return 0;
}

char* XMLNode::ParseDeep( char* p, StrPair* parentEnd )
{
    // This is a recursive method, but thinking about it "at the current level"
    // it is a pretty simple flat list:
    //      <foo/>
    //      <!-- comment -->
    //
    // With a special case:
    //      <foo>
    //      </foo>
    //      <!-- comment -->
    //
    // Where the closing element (/foo) *must* be the next thing after the
    // opening
    // element, and the names must match. BUT the tricky bit is that the closing
    // element will be read by the child.
    //
    // 'endTag' is the end tag for this node, it is returned by a call to a
    //  child.
    // 'parentEnd' is the end tag for the parent, which is filled in and
    //  returned.

    while( p && *p ) {
        XMLNode* node = 0;

        p = _document->Identify( p, &node );
        if ( p == 0 || node == 0 ) {
            break;
        }

        StrPair endTag;
        p = node->ParseDeep( p, &endTag );
        if ( !p ) {
            PRIM_TINYXML_DELETE_NODE( node );
            node = 0;
            if ( !_document->Error() ) {
                _document->SetError( XML_ERROR_PARSING, 0, 0 );
            }
            break;
        }

        // We read the end tag. Return it to the parent.
        if ( node->ToElement() && node->ToElement()->ClosingType() ==
            XMLElement::CLOSING ) {
            if ( parentEnd ) {
                *parentEnd = static_cast<XMLElement*>(node)->_value;
            }
            node->_memPool->SetTracked();//created and then immediately deleted.
            PRIM_TINYXML_DELETE_NODE( node );
            return p;
        }

        // Handle an end tag returned to this level.
        // And handle a bunch of annoying errors.
        XMLElement* ele = node->ToElement();
        if ( ele ) {
            if ( endTag.Empty() && ele->ClosingType() == XMLElement::OPEN ) {
                _document->SetError( XML_ERROR_MISMATCHED_ELEMENT,
                    node->Value(), 0 );
                p = 0;
            }
            else if ( !endTag.Empty() && ele->ClosingType() !=
                XMLElement::OPEN ) {
                _document->SetError( XML_ERROR_MISMATCHED_ELEMENT,
                    node->Value(), 0 );
                p = 0;
            }
            else if ( !endTag.Empty() ) {
                if ( !XMLUtil::StringEqual( endTag.GetStr(), node->Value() )) {
                    _document->SetError( XML_ERROR_MISMATCHED_ELEMENT,
                        node->Value(), 0 );
                    p = 0;
                }
            }
        }
        if ( p == 0 ) {
            PRIM_TINYXML_DELETE_NODE( node );
            node = 0;
        }
        if ( node ) {
            this->InsertEndChild( node );
        }
    }
    return 0;
}

// --------- XMLText ---------- //
char* XMLText::ParseDeep( char* p, StrPair* )
{
    const char* start = p;
    if ( this->CData() ) {
        p = _value.ParseText( p, "]]>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
        if ( !p ) {
            _document->SetError( XML_ERROR_PARSING_CDATA, start, 0 );
        }
        return p;
    }
    else {
        int flags = _document->ProcessEntities() ? StrPair::TEXT_ELEMENT :
            StrPair::TEXT_ELEMENT_LEAVE_ENTITIES;
        if ( _document->WhitespaceMode() == COLLAPSE_WHITESPACE ) {
            flags |= StrPair::COLLAPSE_WHITESPACE;
        }

        p = _value.ParseText( p, "<", flags );
        if ( !p ) {
            _document->SetError( XML_ERROR_PARSING_TEXT, start, 0 );
        }
        if ( p && *p ) {
            return p-1;
        }
    }
    return 0;
}

XMLNode* XMLText::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLText* text = doc->NewText( Value() );
    text->SetCData( this->CData() );
    return text;
}

bool XMLText::ShallowEqual( const XMLNode* compare ) const
{
    return ( compare->ToText() && XMLUtil::StringEqual(
        compare->ToText()->Value(), Value() ));
}

bool XMLText::Accept( XMLVisitor* visitor ) const
{
    return visitor->Visit( *this );
}

// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}

XMLComment::~XMLComment()
{
}

char* XMLComment::ParseDeep( char* p, StrPair* )
{
    // Comment parses as text.
    const char* start = p;
    p = _value.ParseText( p, "-->", StrPair::COMMENT );
    if ( p == 0 ) {
        _document->SetError( XML_ERROR_PARSING_COMMENT, start, 0 );
    }
    return p;
}

XMLNode* XMLComment::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLComment* comment = doc->NewComment( Value() );
    return comment;
}

bool XMLComment::ShallowEqual( const XMLNode* compare ) const
{
    return ( compare->ToComment() && XMLUtil::StringEqual(
        compare->ToComment()->Value(), Value() ));
}

bool XMLComment::Accept( XMLVisitor* visitor ) const
{
    return visitor->Visit( *this );
}

// --------- XMLDeclaration ---------- //

XMLDeclaration::XMLDeclaration( XMLDocument* doc ) : XMLNode( doc )
{
}

XMLDeclaration::~XMLDeclaration()
{
    //printf( "~XMLDeclaration\n" );
}

char* XMLDeclaration::ParseDeep( char* p, StrPair* )
{
    // Declaration parses as text.
    const char* start = p;
    p = _value.ParseText( p, "?>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
    if ( p == 0 ) {
        _document->SetError( XML_ERROR_PARSING_DECLARATION, start, 0 );
    }
    return p;
}

XMLNode* XMLDeclaration::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLDeclaration* dec = doc->NewDeclaration( Value() );
    // fixme: this will always allocate memory. Intern?
    return dec;
}

bool XMLDeclaration::ShallowEqual( const XMLNode* compare ) const
{
    return ( compare->ToDeclaration() && XMLUtil::StringEqual(
        compare->ToDeclaration()->Value(), Value() ));
}

bool XMLDeclaration::Accept( XMLVisitor* visitor ) const
{
    return visitor->Visit( *this );
}

// --------- XMLUnknown ---------- //

XMLUnknown::XMLUnknown( XMLDocument* doc ) : XMLNode( doc )
{
}

XMLUnknown::~XMLUnknown()
{
}

char* XMLUnknown::ParseDeep( char* p, StrPair* )
{
    // Unknown parses as text.
    const char* start = p;

    p = _value.ParseText( p, ">", StrPair::NEEDS_NEWLINE_NORMALIZATION );
    if ( !p ) {
        _document->SetError( XML_ERROR_PARSING_UNKNOWN, start, 0 );
    }
    return p;
}

XMLNode* XMLUnknown::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLUnknown* text = doc->NewUnknown( Value() );
    // fixme: this will always allocate memory. Intern?
    return text;
}

bool XMLUnknown::ShallowEqual( const XMLNode* compare ) const
{
    return ( compare->ToUnknown() && XMLUtil::StringEqual(
        compare->ToUnknown()->Value(), Value() ));
}

bool XMLUnknown::Accept( XMLVisitor* visitor ) const
{
    return visitor->Visit( *this );
}

// --------- XMLAttribute ---------- //

const char* XMLAttribute::Name() const
{
    return _name.GetStr();
}

const char* XMLAttribute::Value() const
{
    return _value.GetStr();
}

char* XMLAttribute::ParseDeep( char* p, bool processEntities )
{
    // Parse using the name rules: bug fix, was using ParseText before
    p = _name.ParseName( p );
    if ( !p || !*p ) {
        return 0;
    }

    // Skip white space before =
    p = XMLUtil::SkipWhiteSpace( p );
    if ( !p || *p != '=' ) {
        return 0;
    }

    ++p;    // move up to opening quote
    p = XMLUtil::SkipWhiteSpace( p );
    if ( *p != '\"' && *p != '\'' ) {
        return 0;
    }

    char endTag[2] = { *p, 0 };
    ++p;    // move past opening quote

    p = _value.ParseText( p, endTag, processEntities ?
        StrPair::ATTRIBUTE_VALUE : StrPair::ATTRIBUTE_VALUE_LEAVE_ENTITIES );
    return p;
}

void XMLAttribute::SetName( const char* n )
{
    _name.SetStr( n );
}

XMLError XMLAttribute::QueryIntValue( int* value ) const
{
    if ( XMLUtil::ToInt( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}

XMLError XMLAttribute::QueryUnsignedValue( unsigned int* value ) const
{
    if ( XMLUtil::ToUnsigned( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}

XMLError XMLAttribute::QueryBoolValue( bool* value ) const
{
    if ( XMLUtil::ToBool( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}

XMLError XMLAttribute::QueryFloatValue( float* value ) const
{
    if ( XMLUtil::ToFloat( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}

XMLError XMLAttribute::QueryDoubleValue( double* value ) const
{
    if ( XMLUtil::ToDouble( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}

void XMLAttribute::SetAttribute( const char* v )
{
    _value.SetStr( v );
}

void XMLAttribute::SetAttribute( int v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( unsigned v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( bool v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( double v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( float v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

// --------- XMLElement ---------- //
XMLElement::XMLElement( XMLDocument* doc ) : XMLNode( doc ),
    _closingType( 0 ),
    _rootAttribute( 0 )
{
}

XMLElement::~XMLElement()
{
    while( _rootAttribute ) {
        XMLAttribute* next = _rootAttribute->_next;
        PRIM_TINYXML_DELETE_ATTRIBUTE( _rootAttribute );
        _rootAttribute = next;
    }
}

XMLAttribute* XMLElement::FindAttribute( const char* name )
{
    XMLAttribute* a = 0;
    for( a=_rootAttribute; a; a = a->_next ) {
        if ( XMLUtil::StringEqual( a->Name(), name ) ) {
            return a;
        }
    }
    return 0;
}

const XMLAttribute* XMLElement::FindAttribute( const char* name ) const
{
    XMLAttribute* a = 0;
    for( a=_rootAttribute; a; a = a->_next ) {
        if ( XMLUtil::StringEqual( a->Name(), name ) ) {
            return a;
        }
    }
    return 0;
}

const char* XMLElement::Attribute( const char* name, const char* value ) const
{
    const XMLAttribute* a = FindAttribute( name );
    if ( !a ) {
        return 0;
    }
    if ( !value || XMLUtil::StringEqual( a->Value(), value )) {
        return a->Value();
    }
    return 0;
}

const char* XMLElement::GetText() const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        return FirstChild()->ToText()->Value();
    }
    return 0;
}

void    XMLElement::SetText( const char* inText )
{
    if ( FirstChild() && FirstChild()->ToText() )
        FirstChild()->SetValue( inText );
    else {
        XMLText*    theText = GetDocument()->NewText( inText );
        InsertFirstChild( theText );
    }
}

void XMLElement::SetText( int v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}

void XMLElement::SetText( unsigned v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}

void XMLElement::SetText( bool v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}

void XMLElement::SetText( float v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}

void XMLElement::SetText( double v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}

XMLError XMLElement::QueryIntText( int* ival ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->ToText()->Value();
        if ( XMLUtil::ToInt( t, ival ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}

XMLError XMLElement::QueryUnsignedText( unsigned* uval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->ToText()->Value();
        if ( XMLUtil::ToUnsigned( t, uval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}

XMLError XMLElement::QueryBoolText( bool* bval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->ToText()->Value();
        if ( XMLUtil::ToBool( t, bval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}

XMLError XMLElement::QueryDoubleText( double* dval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->ToText()->Value();
        if ( XMLUtil::ToDouble( t, dval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}

XMLError XMLElement::QueryFloatText( float* fval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->ToText()->Value();
        if ( XMLUtil::ToFloat( t, fval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}

XMLAttribute* XMLElement::FindOrCreateAttribute( const char* name )
{
    XMLAttribute* last = 0;
    XMLAttribute* attrib = 0;
    for( attrib = _rootAttribute;
            attrib;
            last = attrib, attrib = attrib->_next ) {
        if ( XMLUtil::StringEqual( attrib->Name(), name ) ) {
            break;
        }
    }
    if ( !attrib ) {
        attrib = new (_document->_attributePool.Alloc() ) XMLAttribute();
        attrib->_memPool = &_document->_attributePool;
        if ( last ) {
            last->_next = attrib;
        }
        else {
            _rootAttribute = attrib;
        }
        attrib->SetName( name );
        attrib->_memPool->SetTracked(); // always created and linked.
    }
    return attrib;
}

void XMLElement::DeleteAttribute( const char* name )
{
    XMLAttribute* prev = 0;
    for( XMLAttribute* a=_rootAttribute; a; a=a->_next ) {
        if ( XMLUtil::StringEqual( name, a->Name() ) ) {
            if ( prev ) {
                prev->_next = a->_next;
            }
            else {
                _rootAttribute = a->_next;
            }
            PRIM_TINYXML_DELETE_ATTRIBUTE( a );
            break;
        }
        prev = a;
    }
}

char* XMLElement::ParseAttributes( char* p )
{
    const char* start = p;
    XMLAttribute* prevAttribute = 0;

    // Read the attributes.
    while( p ) {
        p = XMLUtil::SkipWhiteSpace( p );
        if ( !p || !(*p) ) {
            _document->SetError( XML_ERROR_PARSING_ELEMENT, start, Name() );
            return 0;
        }

        // attribute.
        if (XMLUtil::IsNameStartChar( *p ) ) {
            XMLAttribute* attrib = new (_document->_attributePool.Alloc() )
                XMLAttribute();
            attrib->_memPool = &_document->_attributePool;
            attrib->_memPool->SetTracked();

            p = attrib->ParseDeep( p, _document->ProcessEntities() );
            if ( !p || Attribute( attrib->Name() ) ) {
                PRIM_TINYXML_DELETE_ATTRIBUTE( attrib );
                _document->SetError( XML_ERROR_PARSING_ATTRIBUTE, start, p );
                return 0;
            }
            // There is a minor bug here: if the attribute in the source xml
            // document is duplicated, it will not be detected and the
            // attribute will be doubly added. However, tracking the
            // 'prevAttribute'
            // avoids re-scanning the attribute list. Preferring performance for
            // now, may reconsider in the future.
            if ( prevAttribute ) {
                prevAttribute->_next = attrib;
            }
            else {
                _rootAttribute = attrib;
            }
            prevAttribute = attrib;
        }
        // end of the tag
        else if ( *p == '/' && *(p+1) == '>' ) {
            _closingType = CLOSED;
            return p+2; // done; sealed element.
        }
        // end of the tag
        else if ( *p == '>' ) {
            ++p;
            break;
        }
        else {
            _document->SetError( XML_ERROR_PARSING_ELEMENT, start, p );
            return 0;
        }
    }
    return p;
}

//
//  <ele></ele>
//  <ele>foo<b>bar</b></ele>
//
char* XMLElement::ParseDeep( char* p, StrPair* strPair )
{
    // Read the element name.
    p = XMLUtil::SkipWhiteSpace( p );
    if ( !p ) {
        return 0;
    }

    // The closing element is the </element> form. It is
    // parsed just like a regular element then deleted from
    // the DOM.
    if ( *p == '/' ) {
        _closingType = CLOSING;
        ++p;
    }

    p = _value.ParseName( p );
    if ( _value.Empty() ) {
        return 0;
    }

    p = ParseAttributes( p );
    if ( !p || !*p || _closingType ) {
        return p;
    }

    p = XMLNode::ParseDeep( p, strPair );
    return p;
}

XMLNode* XMLElement::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLElement* element = doc->NewElement( Value() );
    for( const XMLAttribute* a=FirstAttribute(); a; a=a->Next() ) {
        element->SetAttribute( a->Name(), a->Value() );
    }
    return element;
}

bool XMLElement::ShallowEqual( const XMLNode* compare ) const
{
    const XMLElement* other = compare->ToElement();
    if ( other && XMLUtil::StringEqual( other->Value(), Value() )) {

        const XMLAttribute* a=FirstAttribute();
        const XMLAttribute* b=other->FirstAttribute();

        while ( a && b ) {
            if ( !XMLUtil::StringEqual( a->Value(), b->Value() ) ) {
                return false;
            }
            a = a->Next();
            b = b->Next();
        }
        if ( a || b ) {
            // different count
            return false;
        }
        return true;
    }
    return false;
}

bool XMLElement::Accept( XMLVisitor* visitor ) const
{
    if ( visitor->VisitEnter( *this, _rootAttribute ) ) {
        for ( const XMLNode* node=FirstChild(); node;
            node=node->NextSibling() ) {
            if ( !node->Accept( visitor ) ) {
                break;
            }
        }
    }
    return visitor->VisitExit( *this );
}

// --------- XMLDocument ----------- //
XMLDocument::XMLDocument( bool processEntities, Whitespace whitespace ) :
    XMLNode( 0 ),
    _writeBOM( false ),
    _processEntities( processEntities ),
    _errorID( XML_NO_ERROR ),
    _whitespace( whitespace ),
    _errorStr1( 0 ),
    _errorStr2( 0 ),
    _charBuffer( 0 )
{
    _document = this;   // avoid warning about 'this' in initializer list
}

XMLDocument::~XMLDocument()
{
    DeleteChildren();
    delete [] _charBuffer;
}

void XMLDocument::Clear()
{
    DeleteChildren();

    _errorID = XML_NO_ERROR;
    _errorStr1 = 0;
    _errorStr2 = 0;

    delete [] _charBuffer;
    _charBuffer = 0;
}

XMLElement* XMLDocument::NewElement( const char* name )
{
    XMLElement* ele = new (_elementPool.Alloc()) XMLElement( this );
    ele->_memPool = &_elementPool;
    ele->SetName( name );
    return ele;
}

XMLComment* XMLDocument::NewComment( const char* str )
{
    XMLComment* comment = new (_commentPool.Alloc()) XMLComment( this );
    comment->_memPool = &_commentPool;
    comment->SetValue( str );
    return comment;
}

XMLText* XMLDocument::NewText( const char* str )
{
    XMLText* text = new (_textPool.Alloc()) XMLText( this );
    text->_memPool = &_textPool;
    text->SetValue( str );
    return text;
}

XMLDeclaration* XMLDocument::NewDeclaration( const char* str )
{
    XMLDeclaration* dec = new (_commentPool.Alloc()) XMLDeclaration( this );
    dec->_memPool = &_commentPool;
    dec->SetValue( str ? str : "xml version=\"1.0\" encoding=\"UTF-8\"" );
    return dec;
}

XMLUnknown* XMLDocument::NewUnknown( const char* str )
{
    XMLUnknown* unk = new (_commentPool.Alloc()) XMLUnknown( this );
    unk->_memPool = &_commentPool;
    unk->SetValue( str );
    return unk;
}

XMLError XMLDocument::Parse( const char* p, unsigned int len )
{
    const char* start = p;
    Clear();

    if ( len == 0 || !p || !*p ) {
        SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
        return _errorID;
    }
    if ( len == (unsigned int)(-1) ) {
        len = (unsigned int)strlen( p );
    }
    _charBuffer = new char[ len+1 ];
    memcpy( _charBuffer, p, len );
    _charBuffer[len] = 0;

    p = XMLUtil::SkipWhiteSpace( p );
    p = XMLUtil::ReadBOM( p, &_writeBOM );
    if ( !p || !*p ) {
        SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
        return _errorID;
    }

    ptrdiff_t delta = p - start;    // skip initial whitespace, BOM, etc.
    ParseDeep( _charBuffer+delta, 0 );
    return _errorID;
}

void XMLDocument::Print( XMLPrinter* streamer ) const
{
    XMLPrinter stdStreamer( stdout );
    if ( !streamer ) {
        streamer = &stdStreamer;
    }
    Accept( streamer );
}

void XMLDocument::SetError( XMLError error, const char* str1, const char* str2 )
{
    _errorID = error;
    _errorStr1 = str1;
    _errorStr2 = str2;
}

void XMLDocument::PrintError() const
{
    if ( _errorID ) {
        static const int LEN = 20;
        char buf1[LEN] = { 0 };
        char buf2[LEN] = { 0 };

        if ( _errorStr1 ) {
            PRIM_TIXML_SNPRINTF( buf1, LEN, "%s", _errorStr1 );
        }
        if ( _errorStr2 ) {
            PRIM_TIXML_SNPRINTF( buf2, LEN, "%s", _errorStr2 );
        }

        printf( "XMLDocument error id=%d str1=%s str2=%s\n",
                _errorID, buf1, buf2 );
    }
}

XMLPrinter::XMLPrinter( bool compact, int depth ) :
    _elementJustOpened( false ),
    _firstElement( true ),
    _depth( depth ),
    _textDepth( -1 ),
    _processEntities( true ),
    _compactMode( compact )
{
    for( int i=0; i<ENTITY_RANGE; ++i ) {
        _entityFlag[i] = false;
        _restrictedEntityFlag[i] = false;
    }
    for( int i=0; i<NUM_ENTITIES; ++i ) {
        if ( entities[i].value < ENTITY_RANGE ) {
            _entityFlag[ (int)entities[i].value ] = true;
        }
    }
    _restrictedEntityFlag[(int)'&'] = true;
    _restrictedEntityFlag[(int)'<'] = true;
    _restrictedEntityFlag[(int)'>'] = true;
    _buffer.Push( 0 );
}

void XMLPrinter::Print( const char* format, ... )
{
    va_list     va;
    va_start( va, format );

    {
#ifdef PRIM_TINYXML_VISUALSTUDIO
        int len = _vscprintf( format, va );
#else
        int len = vsnprintf( 0, 0, format, va );
#endif
        // Close out and re-start the va-args
        va_end( va );
        va_start( va, format );
        char* p = _buffer.PushArr( len ) - 1;
#ifdef PRIM_TINYXML_VISUALSTUDIO
        vsnprintf_s( p, len+1, _TRUNCATE, format, va );
#else
        vsnprintf( p, len+1, format, va );
#endif
    }
    va_end( va );
}

void XMLPrinter::PrintSpace( int depth )
{
    for( int i=0; i<depth; ++i ) {
        Print( "    " );
    }
}

void XMLPrinter::PrintString( const char* p, bool restricted )
{
    // Look for runs of bytes between entities to print.
    const char* q = p;
    const bool* flag = restricted ? _restrictedEntityFlag : _entityFlag;

    if ( _processEntities ) {
        while ( *q ) {
            // Remember, char is sometimes signed.
            if ( *q > 0 && *q < ENTITY_RANGE ) {
                // Check for entities. If one is found, flush
                // the stream up until the entity, write the
                // entity, and keep looking.
                if ( flag[(unsigned)(*q)] ) {
                    while ( p < q ) {
                        Print( "%c", *p );
                        ++p;
                    }
                    for( int i=0; i<NUM_ENTITIES; ++i ) {
                        if ( entities[i].value == *q ) {
                            Print( "&%s;", entities[i].pattern );
                            break;
                        }
                    }
                    ++p;
                }
            }
            ++q;
        }
    }
    // Flush the remaining string. This will be the entire
    // string if an entity wasn't found.
    if ( !_processEntities || (q-p > 0) ) {
        Print( "%s", p );
    }
}

void XMLPrinter::PushHeader( bool writeBOM, bool writeDec )
{
    if ( writeBOM ) {
        static const unsigned char bom[] = { TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1,
            TIXML_UTF_LEAD_2, 0 };
        Print( "%s", bom );
    }
    if ( writeDec ) {
        PushDeclaration( "xml version=\"1.0\"" );
    }
}

void XMLPrinter::OpenElement( const char* name, bool compactMode )
{
    if ( _elementJustOpened ) {
        SealElement();
    }
    _stack.Push( name );

    if ( _textDepth < 0 && !_firstElement && !compactMode ) {
        Print( "\n" );
    }
    if ( !compactMode ) {
        PrintSpace( _depth );
    }

    Print( "<%s", name );
    _elementJustOpened = true;
    _firstElement = false;
    ++_depth;
}

void XMLPrinter::PushAttribute( const char* name, const char* value )
{
    Print( " %s=\"", name );
    PrintString( value, false );
    Print( "\"" );
}

void XMLPrinter::PushAttribute( const char* name, int v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}

void XMLPrinter::PushAttribute( const char* name, unsigned v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}

void XMLPrinter::PushAttribute( const char* name, bool v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}

void XMLPrinter::PushAttribute( const char* name, double v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}

void XMLPrinter::CloseElement( bool compactMode )
{
    --_depth;
    const char* name = _stack.Pop();

    if ( _elementJustOpened ) {
        Print( "/>" );
    }
    else {
        if ( _textDepth < 0 && !compactMode) {
            Print( "\n" );
            PrintSpace( _depth );
        }
        Print( "</%s>", name );
    }

    if ( _textDepth == _depth ) {
        _textDepth = -1;
    }
    if ( _depth == 0 && !compactMode) {
        Print( "\n" );
    }
    _elementJustOpened = false;
}

void XMLPrinter::SealElement()
{
    _elementJustOpened = false;
    Print( ">" );
}

void XMLPrinter::PushText( const char* text, bool cdata )
{
    _textDepth = _depth-1;

    if ( _elementJustOpened ) {
        SealElement();
    }
    if ( cdata ) {
        Print( "<![CDATA[" );
        Print( "%s", text );
        Print( "]]>" );
    }
    else {
        PrintString( text, true );
    }
}

void XMLPrinter::PushText( int value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}

void XMLPrinter::PushText( unsigned value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}

void XMLPrinter::PushText( bool value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}

void XMLPrinter::PushText( float value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}

void XMLPrinter::PushText( double value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}

void XMLPrinter::PushComment( const char* comment )
{
    if ( _elementJustOpened ) {
        SealElement();
    }
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<!--%s-->", comment );
}

void XMLPrinter::PushDeclaration( const char* value )
{
    if ( _elementJustOpened ) {
        SealElement();
    }
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<?%s?>", value );
}

void XMLPrinter::PushUnknown( const char* value )
{
    if ( _elementJustOpened ) {
        SealElement();
    }
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<!%s>", value );
}

bool XMLPrinter::VisitEnter( const XMLDocument& doc )
{
    _processEntities = doc.ProcessEntities();
    if ( doc.HasBOM() ) {
        PushHeader( true, false );
    }
    return true;
}

bool XMLPrinter::VisitEnter( const XMLElement& element,
    const XMLAttribute* attribute )
{
    const XMLElement*   parentElem = element.Parent()->ToElement();
    bool        compactMode = parentElem ? CompactMode(*parentElem) :
        _compactMode;
    OpenElement( element.Name(), compactMode );
    while ( attribute ) {
        PushAttribute( attribute->Name(), attribute->Value() );
        attribute = attribute->Next();
    }
    return true;
}

bool XMLPrinter::VisitExit( const XMLElement& element )
{
    CloseElement( CompactMode(element) );
    return true;
}

bool XMLPrinter::Visit( const XMLText& text )
{
    PushText( text.Value(), text.CData() );
    return true;
}

bool XMLPrinter::Visit( const XMLComment& comment )
{
    PushComment( comment.Value() );
    return true;
}

bool XMLPrinter::Visit( const XMLDeclaration& declaration )
{
    PushDeclaration( declaration.Value() );
    return true;
}

bool XMLPrinter::Visit( const XMLUnknown& unknown )
{
    PushUnknown( unknown.Value() );
    return true;
}

}   // namespace tinyxml2
}}

#endif

#ifdef __clang__
  #pragma clang diagnostic pop
#endif
#endif
