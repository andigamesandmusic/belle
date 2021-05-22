/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#ifndef PRIM_TREE_H
#define PRIM_TREE_H

namespace PRIM_NAMESPACE
{
  /**Red-black tree. This implementation is closely modeled off the 2-3 tree
  presented by Sedgewick in his 2008 red-black tree update concerning 2-3-4
  trees entitled "Left-leaning Red-Black Trees."*/
  template <class K, class V = K>
  class Tree
  {
    //-------------------//
    //Red-Black Constants//
    //-------------------//
    static const bool Red   = true;
    static const bool Black = false;

    //-------------------//
    //Internal Node Class//
    //-------------------//
    class Node
    {
      ///Stores a key and value.
      class KV
      {
        public:

        ///Stores the key in a key-value pair.
        K Key; PRIM_PAD(K)

        ///Stores the value in a key-value pair.
        V Value; PRIM_PAD(V)

        ///Constructs the new key-value.
        KV(const K& NewKey, const V& NewValue) : Key(NewKey), Value(NewValue) {}
      };

      /**Combined pointer to a key object and red-black color of this node. The
      1-bit necessary to store the color is kept in the LSB of the pointer using
      the BooleanPointer class.*/
      BooleanPointer<KV> KeyValueAndColor;

      public:

      ///Gets the color of the node.
      bool GetColor() const {return KeyValueAndColor;}

      ///Sets the color of the node.
      void SetColor(bool NewColor) {KeyValueAndColor.SetBoolean(NewColor);}

      ///Flips the color of the node.
      void FlipColor() {SetColor(not GetColor());}

      ///Gets a reference to the key of the node.
      K& Key() {return KeyValueAndColor->Key;}

      ///Gets a const reference to the key of the node.
      const K& Key() const {return KeyValueAndColor->Key;}

      ///Gets a reference to the value of the node.
      V& Value() {return KeyValueAndColor->Value;}

      ///Gets a const reference to the value of the node.
      const V& Value() const {return KeyValueAndColor->Value;}

      ///Fast swaps the key-value data by pointer.
      void SwapKeyValue(Node* Other)
      {
        BooleanPointer<typename Node::KV>::SwapPointer(KeyValueAndColor,
          Other->KeyValueAndColor);
      }

      ///Node with a key that compares less than the current node key
      Node* Left;

      ///Node with a key that compares greater than the current node key
      Node* Right;

      ///Creates a new node with key and value.
      Node(const K& NewKey, const V& NewValue) :
        KeyValueAndColor(new KV(NewKey, NewValue), Red), Left(0), Right(0) {}

      ///Cleans up the key-value data.
      ~Node()
      {
        //Note: the Tree owns the children nodes and cleans up as necessary.
        KeyValueAndColor.Delete();
      }
    };

    //-------//
    //Members//
    //-------//

    ///Root of the red-black tree
    Node* Root;

    ///Cached number of elements in the tree
    count Elements;

    ///Key to return if the key does not exist.
    K EmptyKeyObject; PRIM_PAD(K)

    ///Value to return if the key does not exist.
    V EmptyValueObject; PRIM_PAD(V)

    //--------------//
    //Helper Methods//
    //--------------//

    ///Returns the value being used for an empty value.
    static V EmptyValue()
    {
      return Nothing<V>();
    }

    ///Returns the value being used for an empty key.
    static K EmptyKey()
    {
      return Nothing<K>();
    }

    ///Deletes an existing node and its children.
    static void Delete(Node* x, count& Elements)
    {
      if(x)
      {
        Delete(x->Left, Elements);
        Delete(x->Right, Elements);
        Elements--;
        delete x;
      }
    }

    ///Creates a new node.
    static Node* New(count& Elements, const K& Key, const V& Value)
    {
      Elements++;
      return new Node(Key, Value);
    }

    ///Comparator to check that a key is less than another.
    static bool LessThan(const K& x, const K& y)
    {
      return x < y;
    }

    ///Comparator to check that a key is greater than another.
    static bool GreaterThan(const K& x, const K& y)
    {
      return x > y;
    }

    ///Comparator to check equivalence of two keys.
    static bool EqualTo(const K& x, const K& y)
    {
      return not LessThan(x, y) and not GreaterThan(x, y);
    }

    ///Returns whether the node is red.
    static bool IsRed(const Node* x)
    {
      return x ? x->GetColor() == Red : false;
    }

    ///Returns whether the node is black.
    static bool IsBlack(const Node* h)
    {
      return not IsRed(h);
    }

    ///Reverse the colors of this node and its children.
    static void FlipColors(Node* h)
    {
      h->FlipColor();
      h->Left->FlipColor();
      h->Right->FlipColor();
    }

    ///Performs a tree rotation.
    static Node* Rotate(Node*& a, Node*& b, Node* h)
    {
      Node* x = a;
      a = b;
      b = h;
      x->SetColor(b->GetColor());
      b->SetColor(Red);
      return x;
    }

    ///Performs a left tree rotation.
    static Node* RotateLeft(Node* h)
    {
      return Rotate(h->Right, h->Right->Left, h);
    }

    ///Performs a right tree rotation.
    static Node* RotateRight(Node* h)
    {
      return Rotate(h->Left, h->Left->Right, h);
    }

    ///Helper function for rotations
    static Node* MoveRedLeft(Node* h)
    {
      FlipColors(h);

      if(IsRed(h->Right->Left))
      {
        h->Right = RotateRight(h->Right);
        h = RotateLeft(h);
        FlipColors(h);
      }

      return h;
    }

    ///Helper function for rotations
    static Node* MoveRedRight(Node* h)
    {
      FlipColors(h);

      if(IsRed(h->Left->Left))
      {
        h = RotateRight(h);
        FlipColors(h);
      }

      return h;
    }

    ///Performs necessary rotations to keep tree balanced.
    static Node* FixUp(Node* h)
    {
      if(IsRed(h->Right))
        h = RotateLeft(h);

      if(IsRed(h->Left) and IsRed(h->Left->Left))
        h = RotateRight(h);

      if(IsRed(h->Left) and IsRed(h->Right))
        FlipColors(h);

      return h;
    }

    ///Gets the const node by key.
    static const Node* Get(const Node* x, const K& Key)
    {
      const Node* h;

      if(not x)
        h = 0;
      else if(LessThan(Key, x->Key()))
        h = Get(x->Left, Key);
      else if(GreaterThan(Key, x->Key()))
        h = Get(x->Right, Key);
      else
        h = x;

      return h;
    }

    ///Gets the node by key.
    static Node* Get(Node* x, const K& Key)
    {
      return const_cast<Node*>(Get(static_cast<const Node*>(x), Key));
    }

    ///Gets the const first node relative to the given node.
    static const Node* First(const Node* x)
    {
      return x->Left ? First(x->Left) : x;
    }

    ///Gets the first node relative to the given node.
    static Node* First(Node* x)
    {
      return const_cast<Node*>(First(static_cast<const Node*>(x)));
    }

    ///Gets the const last node relative to the given node.
    static const Node* Last(const Node* x)
    {
      return x->Right ? Last(x->Right) : x;
    }

    ///Gets the last node relative to the given node.
    static Node* Last(Node* x)
    {
      return const_cast<Node*>(Last(static_cast<const Node*>(x)));
    }

    ///Inserts a node given a key and value assuming incoming node is non-null.
    static Node* InsertAssumeNode(Node* h, count& Elements, const K& Key,
      const V& Value)
    {
      if(LessThan(Key, h->Key()))
        h->Left = Insert(h->Left, Elements, Key, Value);
      else if(GreaterThan(Key, h->Key()))
        h->Right = Insert(h->Right, Elements, Key, Value);
      else
        h->Value() = Value;

      if(IsRed(h->Right))
        h = RotateLeft(h);

      if(IsRed(h->Left) and IsRed(h->Left->Left))
        h = RotateRight(h);

      if(IsRed(h->Left) and IsRed(h->Right))
        FlipColors(h);

      return h;
    }

    ///Inserts a node given a key and value.
    static Node* Insert(Node* h, count& Elements, const K& Key, const V& Value)
    {
      return h ? InsertAssumeNode(h, Elements, Key, Value) :
        New(Elements, Key, Value);
    }

    ///Removes the first node from the given node.
    static Node* RemoveFirst(Node* h, count& Elements)
    {
      Node* x;

      if(h->Left)
      {
        if(IsBlack(h->Left) and IsBlack(h->Left->Left))
          h = MoveRedLeft(h);

        h->Left = RemoveFirst(h->Left, Elements);
        x = FixUp(h);
      }
      else
      {
        Delete(h, Elements);
        x = 0;
      }

      return x;
    }

    ///Removes the last node from the given node.
    static Node* RemoveLast(Node* h, count& Elements)
    {
      Node* x;

      if(IsRed(h->Left))
        h = RotateRight(h);

      if(not h->Right)
      {
        Delete(h, Elements);
        x = 0;
      }
      else
      {
        if(IsBlack(h->Right) and IsBlack(h->Right->Left))
          h = MoveRedRight(h);

        h->Right = RemoveLast(h->Right, Elements);
        x = FixUp(h);
      }

      return x;
    }

    static Node* RemoveLessThan(Node* h, count& Elements, const K& Key)
    {
      if(IsBlack(h->Left) and IsBlack(h->Left->Left))
        h = MoveRedLeft(h);
      h->Left = Remove(h->Left, Elements, Key);
      return FixUp(h);
    }

    static Node* RemoveGreaterOrEqual(Node* h, count& Elements, const K& Key)
    {
      Node* x;
      if(IsRed(h->Left))
        h = RotateRight(h);

      if(EqualTo(Key, h->Key()) and not h->Right)
      {
        Delete(h, Elements);
        x = 0;
      }
      else
      {
        if(IsBlack(h->Right) and IsBlack(h->Right->Left))
          h = MoveRedRight(h);

        if(EqualTo(Key, h->Key()))
        {
          h->SwapKeyValue(First(h->Right));
          h->Right = RemoveFirst(h->Right, Elements);
        }
        else
          h->Right = Remove(h->Right, Elements, Key);

        x = FixUp(h);
      }
      return x;
    }

    ///Removes a node.
    static Node* Remove(Node* h, count& Elements, const K& Key)
    {
      return LessThan(Key, h->Key()) ?
        RemoveLessThan(h, Elements, Key) :
        RemoveGreaterOrEqual(h, Elements, Key);
    }

    //Creates node with a key-value pair from another node.
    static void DeepCopyCreateNode(Node*& Destination, const Node* Source,
      count& Elements)
    {
      Destination = New(Elements, Source->Key(), Source->Value());
      Destination->SetColor(Source->GetColor());
      DeepCopyLinks(Destination, Source, Elements);
    }

    ///Copies linked nodes from another node.
    static void DeepCopyLinks(Node* Destination, const Node* Source,
      count& Elements)
    {
      if(Source->Left)
        DeepCopyCreateNode(Destination->Left, Source->Left, Elements);

      if(Source->Right)
        DeepCopyCreateNode(Destination->Right, Source->Right, Elements);
    }

    ///Deep-copies two trees.
    static void DeepCopy(Tree& Destination, const Tree& Source)
    {
      Destination.RemoveAll();
      if(Source.Root)
        DeepCopyCreateNode(Destination.Root, Source.Root, Destination.Elements);
    }

    ///Determines whether two trees contain identical key-value pairs.
    static bool IsKeyValueIdentical(const Tree& a, const Tree& b)
    {
      bool Identical = true;
      if(a.n() != b.n())
        Identical = false;
      else
      {
        Tree::Iterator It_a;
        Tree::Iterator It_b;
        for(It_a.Begin(a), It_b.Begin(b); It_a.Iterating() and Identical;
          It_a.Next(), It_b.Next())
        {
          if(It_a.Key() != It_b.Key() or It_a.Value() != It_b.Value())
            Identical = false;
        }
      }
      return Identical;
    }

    public:

    ///Creates an empty tree.
    Tree() : Root(0), Elements(0), EmptyKeyObject(EmptyKey()),
      EmptyValueObject(EmptyValue()) {}

    ///Removes all elements from the tree and destroys the tree.
    ~Tree() {RemoveAll();}

    ///Copy-constructor that creates a deep-copy of another tree.
    Tree(const Tree& Other) : Root(0), Elements(0), EmptyKeyObject(EmptyKey()),
      EmptyValueObject(EmptyValue())
    {
      DeepCopy(*this, Other);
    }

    ///Assignment operator creates a deep-copy of another tree.
    Tree& operator = (const Tree& Other)
    {
      DeepCopy(*this, Other);
      return *this;
    }

    ///Returns whether the trees have identical key-value pairs.
    bool operator == (const Tree& Other) const
    {
      return IsKeyValueIdentical(*this, Other);
    }

    ///Returns whether the trees do not have identical key-value pairs.
    bool operator != (const Tree& Other) const
    {
      return not IsKeyValueIdentical(*this, Other);
    }

    ///Determines whether the key exists in the tree.
    bool Contains(const K& Key) const
    {
      return Get(Root, Key);
    }

    ///Returns whether the tree is empty.
    bool Empty() const
    {
      return not Root;
    }

    /**Gets the value at a given key. If the key does not exist, the Nothing
    value for the value type is returned.*/
    const V& Get(const K& Key) const
    {
      const Node* h = Get(Root, Key);
      return h ? h->Value() : EmptyValueObject;
    }

    ///Gets the key of the first element.
    const K& First() const
    {
      return Root ? First(Root)->Key() : EmptyKeyObject;
    }

    ///Gets the key of the last element.
    const K& Last() const
    {
      return Root ? Last(Root)->Key() : EmptyKeyObject;
    }

    ///Sets a key-value.
    void Set(const K& Key, const V& Value)
    {
      Root = Insert(Root, Elements, Key, Value);
      Root->SetColor(Black);
    }

    ///Lazily sets a key-value.
    V& Set(const K& Key)
    {
      Node* h = Get(Root, Key);
      return h ? h->Value() :
        (Set(Key, EmptyValue()), Get(Root, Key)->Value());
    }

    /**Returns the value at a given key. If the key does not exist, the Nothing
    value for the value type is returned.*/
    const V& operator [] (const K& Key) const {return Get(Key);}

    /**Lazily sets a key-value. The initial value for new keys is the Nothing
    value for the value type.*/
    V& operator [] (const K& Key) {return Set(Key);}

    ///Returns the number of elements in the tree.
    count n() const
    {
      return Elements;
    }

    ///Removes the first element.
    void RemoveFirst()
    {
      if(Root && (Root = RemoveFirst(Root, Elements)) != 0)
        Root->SetColor(Black);
    }

    ///Removes the last element.
    void RemoveLast()
    {
      if(Root && (Root = RemoveLast(Root, Elements)) != 0)
        Root->SetColor(Black);
    }

    ///Removes all of the elements from the tree.
    void RemoveAll()
    {
      Delete(Root, Elements);
      Root = 0;
    }

    ///Removes an element by key. If the key does not exist, no change is made.
    void Remove(const K& Key)
    {
      //Make sure the element exists before attempting to remove it.
      if(Get(Root, Key) && (Root = Remove(Root, Elements, Key)) != 0)
        Root->SetColor(Black);
    }

    private:

    ///Calculates the actual height of the tree.
    count CalculateHeight(Node* n, count Level) const
    {
      if(n)
      {
        Level++;
        Level = Max(
          CalculateHeight(n->Left, Level),
          CalculateHeight(n->Right, Level));
      }
      return Level;
    }

    public:

    ///Calculates the actual height of the tree.
    count CalculateHeight() const {return CalculateHeight(Root, 0);}

    ///Gets the maximum height of the tree based off the number of elements.
    count MaximumHeight() const
    {
      return count(Ceiling(Log2(number(Elements) + 1.f) * 2.f));
    }

    //Give the iterator direct access to the Tree.
    friend class Iterator;

    ///Sequential iterator for the Tree class.
    class Iterator
    {
      ///A stack allocated to the maximum possible height of the tree.
      const Node** Stack;

      ///Stores the current node being visited.
      const Node* CurrentNode;

      ///Stores the last node in the tree.
      const Node* LastNode;

      ///Visitor level used during iteration.
      count VisitorLevel;

      ///Empty key to return when not iterating.
      K EmptyKeyObject; PRIM_PAD(K)

      ///Empty value to return when not iterating.
      V EmptyValueObject; PRIM_PAD(V)

      ///Cleans up the iterator completely.
      void Reset()
      {
        delete [] Stack;
        Stack = 0;
        CurrentNode = 0;
        LastNode = 0;
        VisitorLevel = -2;
      }

      ///Initializes the stack with a given height.
      void CreateStackWithHeight(count StackHeight)
      {
        Stack = new const Node*[StackHeight];
        Memory::Clear(Stack, StackHeight);
      }

      public:

      ///Creates an empty iterator.
      Iterator() : Stack(0), CurrentNode(0), LastNode(0), VisitorLevel(-2),
        EmptyKeyObject(EmptyKey()), EmptyValueObject(EmptyValue()) {}

      ///Begins iterating a given tree and returns the key of the first element.
      void Begin(const Tree& T)
      {
        Reset();
        CreateStackWithHeight(T.MaximumHeight());

        //Travel to the left-most node.
        if(T.Root)
        {
          VisitorLevel++;
          Node* LeftMostNode = T.Root;
          while(LeftMostNode)
          {
            Stack[++VisitorLevel] = LeftMostNode;
            LeftMostNode = LeftMostNode->Left;
          }
          CurrentNode = Stack[VisitorLevel];

          //Determine the last node.
          LastNode = T.Root;
          while(LastNode->Right)
            LastNode = LastNode->Right;

          //If the root is the only node then end the traversal immediately.
          if(CurrentNode == LastNode)
            Stack[VisitorLevel--] = 0;
        }
      }

      ///Returns true if there are no more nodes to visit.
      bool Ending() const
      {
        return VisitorLevel < -1;
      }

      ///Returns true if there are still nodes to visit.
      bool Iterating() const
      {
        return not Ending();
      }

      /**Returns the next node. If there are no more nodes to visit, the key
      will be set to the nothing value for its type.*/
      void Next()
      {
        /*If a node was found decrement the visitor level to indicate that the
        iteration has ended.*/
        if(VisitorLevel == -1)
        {
          VisitorLevel--;
          CurrentNode = 0;
        }
        else if(VisitorLevel < -1)
          return;

        //Traverse the graph while there is a next key and a non-empty stack.
        bool FoundNextKey = false;
        bool PoppingUpRightSide = false;
        while(not FoundNextKey and VisitorLevel >= 0)
        {
          if(Stack[VisitorLevel]->Right and not PoppingUpRightSide)
          {
            //The next node is the the left-most child node off of this node.
            {
              Node* LeftMost = Stack[VisitorLevel]->Right;
              while(LeftMost)
              {
                Stack[++VisitorLevel] = LeftMost;
                LeftMost = LeftMost->Left;
              }
            }

            //Update the key for the left-most node.
            CurrentNode = Stack[VisitorLevel];
            FoundNextKey = true;

            /*If last node was visited, then the iteration is finished and the
            whole stack can be popped.*/
            if(CurrentNode == LastNode)
              while(VisitorLevel >= 0)
                Stack[VisitorLevel--] = 0;
          }
          else
          {
            //Determine the direction we came from.
            bool CameFromLeft =
              Stack[VisitorLevel] == Stack[VisitorLevel - 1]->Left;

            //Pop one off the stack.
            Stack[VisitorLevel--] = 0;

            /*If the child node was branched from the left side, then visit the
            parent.*/
            if(CameFromLeft)
            {
              //Update the key for the parent node.
              CurrentNode = Stack[VisitorLevel];
              FoundNextKey = true;

              /*If last node was visited, then the iteration is finished and the
              whole stack can be popped.*/
              if(Stack[VisitorLevel] == LastNode)
                while(VisitorLevel >= 0)
                  Stack[VisitorLevel--] = 0;
            }
            else
              PoppingUpRightSide = true;
          }
        }
      }

      /**Returns the value associated with the current key. If there is no
      current key, then the nothing value for the value type is returned.*/
      const K& Key() const
      {
        return CurrentNode ? CurrentNode->Key() : EmptyKeyObject;
      }

      /**Returns the value associated with the current key. If there is no
      current key, then the nothing value for the value type is returned.*/
      const V& Value() const
      {
        return CurrentNode ? CurrentNode->Value() : EmptyValueObject;
      }

      ///Cleans up the iterator.
      ~Iterator() {Reset();}
    };

    ///Returns an ordered-array of keys from this tree.
    void Keys(Array<K>& KeysToReturn) const
    {
      KeysToReturn.n(n());
      Iterator It;
      count i = 0;
      for(It.Begin(*this); It.Iterating(); It.Next())
        KeysToReturn[i++] = It.Key();
    }

    ///Returns an ordered-array of keys from this tree.
    Array<K> Keys() const
    {
      Array<K> KeyArray;
      Keys(KeyArray);
      return KeyArray;
    }

    ///Returns a key-ordered array of values from this tree.
    void Values(Array<V>& ValuesToReturn) const
    {
      ValuesToReturn.n(n());
      Iterator It;
      count i = 0;
      for(It.Begin(*this); It.Iterating(); It.Next())
        ValuesToReturn[i++] = It.Value();
    }

    ///Returns a key-ordered array of values from this tree.
    Array<V> Values() const
    {
      Array<V> ValueArray;
      Values(ValueArray);
      return ValueArray;
    }

    ///Prunes any keys with nothing values. Returns the number of items pruned.
    count Prune()
    {
      //Build a list of keys to prune.
      List<K> KeysToPrune;
      Iterator It;
      for(It.Begin(*this); It.Iterating(); It.Next())
      {
        if(Nothing<V>() == It.Value())
          KeysToPrune.Add() = It.Key();
      }

      //Prune the keys from the list.
      count KeysPruned = KeysToPrune.n();
      while(KeysToPrune.n())
        Remove(KeysToPrune.Pop());

      return KeysPruned;
    }
  };
}
#endif
