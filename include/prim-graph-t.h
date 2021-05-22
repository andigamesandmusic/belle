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

#ifndef PRIM_INCLUDE_GRAPH_T_H
#define PRIM_INCLUDE_GRAPH_T_H

namespace PRIM_NAMESPACE
{
  ///Naive implementation of a priority queue running in O(n) time.
  template <class Element, class Priority>
  class PriorityQueue
  {
    class PrioritizedElement
    {
      public:

      Element E; PRIM_PAD(Element)
      Priority P; PRIM_PAD(Priority)

      bool operator < (const PrioritizedElement& Other) const
      {
        return P < Other.P or (not (P > Other.P) and E < Other.E);
      }

      bool operator > (const PrioritizedElement& Other) const
      {
        return P > Other.P or (not (P < Other.P) and E > Other.E);
      }

      bool operator == (const PrioritizedElement& Other) const
      {
        return not (*this < Other or *this > Other);
      }
    };

    List<PrioritizedElement> PriorityList;

    public:

    ///Adds an element with a given priority.
    void AddWithPriority(const Element& E, Priority P)
    {
      PrioritizedElement PE;
      PE.P = P, PE.E = E;
      bool Inserted = false;
      for(count i = 0; i < PriorityList.n() and not Inserted; i++)
        if(PE < PriorityList[i] or PE == PriorityList[i])
          PriorityList.InsertBefore(PE, i), Inserted = true;
      if(not Inserted)
        PriorityList.Append(PE);
    }

    ///Changes the priority of an element.
    void ChangePriority(const Element& E, Priority P)
    {
      bool Removed = false;
      for(count i = 0; i < PriorityList.n() and not Removed; i++)
        if(PriorityList[i].E == E)
          PriorityList.Remove(i), Removed = true;
      AddWithPriority(E, P);
    }

    ///Returns the element with the least-valued priority and removes it.
    Element ExtractMinimum()
    {
      Element E = PriorityList.a().E;
      PriorityList.Remove(0);
      return E;
    }

    ///Returns whether the priority queue still has elements.
    bool HasElements() const
    {
      return PriorityList.n();
    }
  };

  /**Labeled multidigraph using templated labels. Const for a graph means that
  the nodes and edges may not be altered and their labels must not be altered.
  The graph owns all the nodes and edges. Formally, there is no difference
  between a node and an edge; they are both represented by the Object dependent
  class. A node is an Object that stores connected edges and an edge is an
  Object that stores its connected nodes. All nodes and edges are of the same
  type and rely on expressive labels rather than node subclassing to
  differentiate types of node information.*/
  template<typename L>
  class GraphT
  {
    public:

    //----------------------------//
    //Construction and Destruction//
    //----------------------------//

    ///Creates an empty (order-zero) graph.
    GraphT() {}

    ///Destructor clears the graph.
    ~GraphT() {Clear();}

    ///Using the copy-constructor on a graph is not supported.
    GraphT(const GraphT&) PRIM_11_DELETE_DEFAULT;

    ///Graph assignment is not supported.
    GraphT& operator = (const GraphT&) PRIM_11_DELETE_DEFAULT;

    //-------//
    //Objects//
    //-------//

    /**Represents either a node or an edge. They are combined into one class to
    ease interdependency problems. Since both nodes and edges may contain labels
    they are very similar.*/
    class Object : public L
    {
      //Give the Graph class access to this class.
      template <class U> friend class GraphT;

      ///Connected edges in the case of the object being a node.
      Tree<Pointer<Object>, bool> Edges;

      /* #unresolved : May be able to combine Self and From if From/To are
      changed to a weak auto-pointer. Not sure if this would interfere with
      garbage collection or referencing-counting.*/

      ///Pointers to nodes in the case of the object being an edge.
      Pointer<Object> From, To;

      /**Maintain a weak pointer to self. This will allow a method to retrieve
      the original auto-pointer handle. It needs to be weak so that it does not
      prevent itself from being garbage collected.*/
      typename Pointer<Object>::Weak Self;

      /*Only GraphT may construct an Object.*/ Object() : Label(*this) {}

      /**Returns a copy of the array with unconst pointers. This is used in
      special circumstances to prevent code duplication between const and
      non-const methods.*/
      Array<Pointer<Object> > ForceUnconst(
        const Array<Pointer<const Object> >& ConstArray)
      {
        Array<Pointer<Object> > UnconstArray(ConstArray.n());
        for(count i = 0; i < ConstArray.n(); i++)
          UnconstArray[i] = ForceUnconst(ConstArray[i]);
        return UnconstArray;
      }

      /**Returns an unconst version of the pointer. This is used in special
      circumstances to prevent code duplication between const and non-const
      methods.*/
      Pointer<Object> ForceUnconst(const Pointer<const Object>& ConstPointer)
      {
        return reinterpret_cast<const Pointer<Object>&>(ConstPointer);
      }

      public:

      ///Reference to the label.
      L& Label;

      ///Returns whether this object is an edge.
      bool IsEdge() const {return From and To;}

      ///Returns whether this object is a node.
      bool IsNode() const {return not From and not To;}

      ///Constant key-value lookup
      template <class U> U Get(const U& K) const {return Label.Get(K);}

      ///Mutable key-value lookup
      template <class U> U& Set(const U& K) {return Label.Set(K);}

      ///Constant key-value lookup overload
      String Get(const ascii* K) const {return Label.Get(K);}

      ///Mutable key-value lookup overload
      String& Set(const ascii* K) {return Label.Set(K);}

      ///Mutable key-value lookup overload
      void Set(const ascii* K, const ascii* V) {Label.Set(K, V);}

      ///String conversion
      operator String() const {return L::operator String();}

      //----//
      //Edge//
      //----//

      Pointer<Object> Head()
      {
        if(not IsEdge())
          return Pointer<Object>();
        return To;
      }

      Pointer<const Object> Head() const
      {
        if(not IsEdge())
          return Pointer<const Object>();
        return To;
      }

      Pointer<Object> Tail()
      {
        if(not IsEdge())
          return Pointer<Object>();
        return From;
      }

      Pointer<const Object> Tail() const
      {
        if(not IsEdge())
          return Pointer<const Object>();
        return From;
      }

      //---------//
      //Traversal//
      //---------//

      /**Gets the successor node following an edge that matches the filter. If
      there is more than one edge that matches the filter, then null is returned
      and the Children method must be used instead.*/
      Pointer<const Object> Next(const L& Filter, bool ReturnEdge = false) const
      {
        if(not IsNode()) return Pointer<Object>();

        /*Iterate through each edge and look for a forwards one whose label is
        edge-equivalent to the filter.*/
        Pointer<const Object> Result;
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(Edges); It.Iterating(); It.Next())
        {
          Pointer<const Object> Edge = It.Key();
          if(Edge->From == Self and Edge->Label.EdgeEquivalent(Filter))
          {
            /*If multiple matching nodes are found, return null. Caller must use
            the Children() method instead.*/
            if(Result)
              return Pointer<Object>();
            Result = ReturnEdge ? Edge : Edge->To.Const();
          }
        }

        //Return the node found via the edge filter.
        return Result;
      }

      /**Gets the successor node following an edge that matches the filter. If
      there is more than one edge that matches the filter, then null is returned
      and the Children method must be used instead.*/
      Pointer<Object> Next(const L& Filter, bool ReturnEdge = false)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Next(Filter,
          ReturnEdge));
      }

      /**Gets the predecessor node following an edge that matches the filter. If
      there is more than one edge that matches the filter, then null is returned
      and the Children method must be used instead.*/
      Pointer<const Object> Previous(const L& Filter,
        bool ReturnEdge = false) const
      {
        if(not IsNode()) return Pointer<Object>();

        /*Iterate through each edge and look for a backwards one whose label is
        edge-equivalent to the filter.*/
        Pointer<const Object> Result;
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(Edges); It.Iterating(); It.Next())
        {
          Pointer<const Object> Edge = It.Key();
          if(Edge->To == Self and Edge->Label.EdgeEquivalent(Filter))
          {
            /*If multiple matching nodes are found, return null. Caller must use
            the Children() method instead.*/
            if(Result)
              return Pointer<const Object>();
            Result = ReturnEdge ? Edge : Edge->From.Const();
          }
        }

        //Return the node found via the edge filter.
        return Result;
      }

      /**Gets the predecessor node following an edge that matches the filter. If
      there is more than one edge that matches the filter, then null is returned
      and the Children method must be used instead.*/
      Pointer<Object> Previous(const L& Filter, bool ReturnEdge = false)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Previous(Filter,
          ReturnEdge));
      }

      ///Finds the first instance of a node in a series that matches a filter.
      Pointer<const Object> First(const L& Filter) const
      {
        Tree<Pointer<const Object>, bool> Visited;
        Pointer<const Object> p = Self, r;
        if(IsNode()) do Visited[r = p] = true; while(
          (p = r->Previous(Filter)) and not Visited.Contains(p));
        return r;
      }

      ///Finds the first instance of a node in a series that matches a filter.
      Array<Pointer<Object> > First(const L& Filter)
      {
        return ForceUnconst(static_cast<const Object&>(*this).First(Filter));
      }

      ///Finds the last instance of a node in a series that matches a filter.
      Pointer<const Object> Last(const L& Filter) const
      {
        Tree<Pointer<const Object>, bool> Visited;
        Pointer<const Object> n = Self, r;
        if(IsNode()) do Visited[r = n] = true; while(
          (n = r->Next(Filter)) and not Visited.Contains(n));
        return r;
      }

      ///Finds the last instance of a node in a series that matches a filter.
      Array<Pointer<Object> > Last(const L& Filter)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Last(Filter));
      }

      /**Returns the series of a node by following edges that match the filter.
      If Backup is true, then traversal will start from the first in the
      series. Otherwise, traversal starts from the current node.*/
      Array<Pointer<const Object> > Series(const L& Filter,
        bool Backup = true) const
      {
        //Create an array of the series.
        Array<Pointer<const Object> > SeriesNodes;
        Tree<Pointer<const Object>, bool> Visited;

        if(not IsNode()) return Array<Pointer<const Object> >();

        //Back the node up as far as it can go.
        Pointer<const Object> Current = Backup ? First(Filter) : Self.Const();

        //Traverse series add each element to the array.
        SeriesNodes.Add() = Current, Visited[Current] = true;
        while((Current = Current->Next(Filter)) and
          not Visited.Contains(Current))
            Visited[SeriesNodes.Add() = Current] = true;

        return SeriesNodes;
      }

      /**Returns the series of a node by following edges that match the filter.
      If Backup is true, then traversal will start from the first in the
      series. Otherwise, traversal starts from the current node.*/
      Array<Pointer<Object> > Series(const L& Filter, bool Backup = true)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Series(
          Filter, Backup));
      }

      ///Returns the children of a node following edges that match the filter.
      Array<Pointer<const Object> > Children(const L& Filter,
        bool ReturnEdges = false) const
      {
        Array<Pointer<const Object> > ChildNodes;
        if(IsEdge()) return ChildNodes;

        /*Iterate through each edge and look for forwards edges whose label is
        edge-equivalent to the filter.*/
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(Edges); It.Iterating(); It.Next())
        {
          Pointer<const Object> Edge = It.Key();
          if(Edge->From == Self and Edge->Label.EdgeEquivalent(Filter))
            ChildNodes.Add(ReturnEdges ? Edge : Edge->To.Const());
        }

        //No edge matched the filter so return an empty node.
        return ChildNodes;
      }

      ///Returns the children of a node following edges that match the filter.
      Array<Pointer<Object> > Children(const L& Filter,
        bool ReturnEdges = false)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Children(
          Filter, ReturnEdges));
      }

      ///Returns the parents of a node following edges that match the filter.
      Array<Pointer<const Object> > Parents(const L& Filter,
        bool ReturnEdges = false) const
      {
        Array<Pointer<const Object> > ParentNodes;
        if(IsEdge()) return ParentNodes;

        /*Iterate through each edge and look for forwards edges whose label is
        edge-equivalent to the filter.*/
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(Edges); It.Iterating(); It.Next())
        {
          Pointer<const Object> Edge = It.Key();
          if(Edge->To == Self and Edge->Label.EdgeEquivalent(Filter))
            ParentNodes.Add(ReturnEdges ? Edge : Edge->From.Const());
        }

        //No edge matched the filter so return an empty node.
        return ParentNodes;
      }

      ///Returns the parents of a node following edges that match the filter.
      Array<Pointer<Object> > Parents(const L& Filter, bool ReturnEdges = false)
      {
        return ForceUnconst(static_cast<const Object&>(*this).Parents(
          Filter, ReturnEdges));
      }
    };

    public:

    //---------------//
    //Nodes and Edges//
    //---------------//

    /**Adds a node to the graph. The node is isolated until it is connected to
    other nodes. If this is the first node in the graph, then the root of the
    graph is set to this node. A pointer to the new node is returned.*/
    Pointer<Object> Add()
    {
      //Create a new node.
      Pointer<Object> n = new Object;

      /*Cache the auto-pointer handle in the object so that it can recover an
      auto-pointer handle to itself.*/
      n->Self = n;

      //Set the node as root if it is the first in the graph.
      if(NodeTree.Empty())
        RootNode = n;

      //Add the node to the node tree.
      NodeTree[n] = true;

      //Return the new node.
      return n;
    }

    /**Connects two nodes with a directed edge from x to y and returns the edge.
    This method will return a valid pointer if x and y are both nodes that
    belong to the graph.*/
    Pointer<Object> Connect(Pointer<Object> x, Pointer<Object> y)
    {
      //Make sure the nodes both belong to this graph.
      if(not Belongs(x) or not Belongs(y) or x->IsEdge() or y->IsEdge())
        return 0;

      //Create the edge.
      Pointer<Object> e(new Object);
      e->From = x;
      e->To = y;

      //Add a reference to the edge in both nodes.
      x->Edges[e] = true;
      if(y != x)
        y->Edges[e] = true;

      //Return the new edge.
      return e;
    }

    /**Disconnects the node or the edge. If a node, it causes the node to become
    isolated. If the node does not belong to the graph, then this has no effect.
    If an edge, it removes the edge from the nodes it is attached to.*/
    void Disconnect(Pointer<Object> n)
    {
      if(not n) return;

      if(n->IsEdge())
      {
        //Make sure the edge exists and that the nodes belong to this graph.
        if(not n or not Belongs(n->From) or not Belongs(n->To))
          return;

        /*Disconnect the two nodes sharing the edge. This pops an element off
        the current tree.*/
        n->From->Edges.Remove(n);
        if(n->To != n->From)
          n->To->Edges.Remove(n);
      }
      else
      {
        //Ensure the node belongs to this graph.
        if(not Belongs(n)) return;

        /*Iterate through each edge in the node and disconnect it. Note that the
        iteration must start from the top of the stack because it is popping
        edges off the tree.*/
        while(not n->Edges.Empty())
        {
          //Get a pointer to the current edge.
          Pointer<Object> e = n->Edges.Last();

          /*Disconnect the two nodes sharing the edge. This pops an element off
          the current tree.*/
          e->From->Edges.Remove(e);
          if(e->To != e->From)
            e->To->Edges.Remove(e);

          //Once e goes out of scope, the edge will be deleted.
        }
      }
    }

    /**Disconnects a node or edge from the graph and removes it. If the node
    does not belong to the graph, then this has no effect. For edges this has
    the same effect as Disconnect().*/
    void Remove(Pointer<Object> n)
    {
      //Ensure the node or edge belongs to this graph.
      if(not Belongs(n)) return;

      //Remember whether this was a node.
      bool WasNode = n->IsNode();

      //Disconnect the node or edge first.
      Disconnect(n);

      //If it is a node, then remove its entry in the node tree.
      if(WasNode)
        NodeTree.Remove(n);

      //Once the last pointer to n goes out of scope, the node is deleted.
    }

    //-------//
    //General//
    //-------//

    ///Entirely clears the graph structure and its contents.
    void Clear()
    {
      //Disconnect each node from the graph.
      while(not NodeTree.Empty())
      {
        Pointer<Object> n = NodeTree.Last();

        /*By zeroing out the label pointer, it will automatically destroy the
        label. This is necessary because the label could contain auto-pointers
        to nodes in the graph, creating a mutual reference that could cause a
        memory leak. This ensures at the very least that no information on the
        graph is preventing the graph from being garbage collected.*/
        static_cast<L&>(*n) = L();

        //Disconnect the node from any other nodes, leaving it isolated.
        Disconnect(n);

        NodeTree.RemoveLast();
      }
      RootNode = Pointer<Object>();
    }

    ///Returns whether a node or an edge belongs to the graph.
    bool Belongs(Pointer<const Object> n) const
    {
      /*Check for null. Technically there should not be any null pointers in the
      node tree, but this saves having to check the tree.*/
      if(not n)
        return false;

      if(n->IsNode())
      {
        /*Look for the node in tree and report whether it exists.
        #voodoo Note the forced const object conversion is to enable looking up
        in the non-const object tree.*/
        return NodeTree[*reinterpret_cast<Pointer<Object>*>(&n)];
      }

      //Determine if the edge nodes belong to the graph.
      return Belongs(n->To) and Belongs(n->From);
    }

    /**Promotes a const node or edge pointer to a mutable one. This is only
    allowed when calling this method on a mutable graph. If the node or edge
    does not belong to the graph, then an empty pointer is returned.*/
    Pointer<Object> Promote(Pointer<const Object> x)
    {
      /*#voodoo Forced const object conversion is valid since the object is
      guaranteed to be owned by the current graph and the current method has
      mutable access to the graph.*/
      return Belongs(x) ? *reinterpret_cast<Pointer<Object>*>(&x) :
        Pointer<Object>();
    }

    ///Returns an array of all the nodes in the graph.
    Sortable::Array<Pointer<Object> > Nodes()
    {
      //Initialize a node array and size it to match the node tree size.
      Sortable::Array<Pointer<Object> >
        NodeArray(NodeTree.n());

      //Fill a node array with the keys of the node tree.
      typename Tree<Pointer<Object>, bool>::Iterator It;
      count i = 0;
      for(It.Begin(NodeTree); It.Iterating(); It.Next())
      {
        Pointer<Object> k = It.Key();
        NodeArray[i++] = k;
      }

      //Sort the array before returning (should already be sorted).
      NodeArray.Sort();

      //Return the node array containing all the nodes in the graph.
      return NodeArray;
    }

    ///Returns an array of all the nodes in the graph.
    Sortable::Array<Pointer<const Object> > Nodes() const
    {
      //Initialize a node array and size it to match the node tree size.
      Sortable::Array<Pointer<const Object> >
        NodeArray(NodeTree.n());

      //Fill a node array with the keys of the node tree.
      typename Tree<Pointer<Object>, bool>::Iterator It;
      count i = 0;
      for(It.Begin(NodeTree); It.Iterating(); It.Next())
      {
        Pointer<const Object> k = It.Key();
        NodeArray[i++] = k;
      }

      //Sort the array before returning (should already be sorted).
      NodeArray.Sort();

      //Return the node array containing all the nodes in the graph.
      return NodeArray;
    }

    ///Returns an array of all the edges in the graph.
    Sortable::Array<Pointer<const Object> > Edges() const
    {
      //Create an edge tree with all the edges in the graph.
      Tree<Pointer<Object>, bool> EdgeTree;

      //Populate an edge tree with all the edges in the graph.
      {
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(NodeTree); It.Iterating(); It.Next())
        {
          Pointer<Object> Current = It.Key();
          typename Tree<Pointer<Object>, bool>::Iterator Jt;
          for(Jt.Begin(Current->Edges); Jt.Iterating(); Jt.Next())
          {
            Pointer<Object> Edge = Jt.Key();
            EdgeTree[Edge] = true;
          }
        }
      }

      //Initialize a edge array and size it to match the edge tree size.
      Sortable::Array<Pointer<const Object> >
        EdgeArray(EdgeTree.n());

      //Fill a edge array with the keys of the node tree.
      {
        typename Tree<Pointer<Object>, bool>::Iterator It;
        count i = 0;
        for(It.Begin(EdgeTree); It.Iterating(); It.Next())
        {
          Pointer<const Object> Edge = It.Key();
          EdgeArray[i++] = Edge;
        }
      }

      //Sort the array before returning.
      EdgeArray.Sort();

      //Return the edge array containing all the edges in the graph.
      return EdgeArray;
    }

    //---------//
    //Traversal//
    //---------//

    /**Gets the successor node following an edge that matches the filter. If
    there is more than one edge that matches the filter, then null is returned
    and the Children method must be used instead.*/
    Pointer<Object> Next(Pointer<Object> Node, const L& Filter)
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Pointer<Object>();
      return Node->Next(Filter);
    }

    /**Gets the successor node following an edge that matches the filter. If
    there is more than one edge that matches the filter, then null is returned
    and the Children method must be used instead.*/
    Pointer<const Object> Next(Pointer<const Object> Node, const L& Filter)
      const
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Pointer<const Object>();
      return Node->Next(Filter);
    }

    /**Gets the predecessor node following an edge that matches the filter. If
    there is more than one edge that matches the filter, then null is returned
    and the Children method must be used instead.*/
    Pointer<Object> Previous(Pointer<Object> Node, const L& Filter)
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Pointer<Object>();
      return Node->Previous(Filter);
    }

    /**Gets the predecessor node following an edge that matches the filter. If
    there is more than one edge that matches the filter, then null is returned
    and the Children method must be used instead.*/
    Pointer<const Object> Previous(Pointer<const Object> Node, const L& Filter)
      const
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Pointer<const Object>();
      return Node->Previous(Filter);
    }

    ///Returns the series of a node by following edges that match the filter.
    Array<Pointer<Object> > Series(Pointer<Object> Node, const L& Filter)
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Array<Pointer<Object> >();
      return Node->Series(Filter);
    }

    /**Returns the series of a node by following edges that match the filter.
    Note this does not currently detect cycles.*/
    Array<Pointer<const Object> > Series(Pointer<const Object> Node,
      const L& Filter) const
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Array<Pointer<const Object> >();
      return Node->Series(Filter);
    }

    /**Returns the series of a node by following edges that match the filter.
    Note this does not currently detect cycles.*/
    Array<Pointer<Object> > Children(Pointer<Object> Node,
      const L& Filter)
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Array<Pointer<Object> >();
      return Node->Children(Filter);
    }

    ///Returns the children of a node by following edges that match the filter.
    Array<Pointer<const Object> > Children(Pointer<const Object> Node,
      const L& Filter) const
    {
      //Make sure the starting object is a node in the graph.
      if(not Belongs(Node) or not Node->IsNode())
        return Array<Pointer<const Object> >();
      return Node->Children(Filter);
    }

    //----//
    //Root//
    //----//

    /**Sets a new root node to the graph. If the node does not belong to the
    graph then no change is made.*/
    void Root(Pointer<Object> NewRoot)
    {
      //Ensure the node belongs to this graph.
      if(not Belongs(NewRoot)) return;

      //Set the new root node.
      RootNode = NewRoot;
    }

    ///Returns the root node of the graph if it has been set.
    Pointer<Object> Root() {return RootNode;}

    ///Returns the const root node of the graph if it has been set.
    Pointer<const Object> Root() const {return RootNode;}

    //---------------//
    //Cycle Detection//
    //---------------//

    ///Returns whether the graph a cycle following the given edge filter.
    bool IsCyclic(const L& Filter) const
    {
      return Cycle(Filter).n();
    }

    ///Returns the first found cycle of the given edge filter if one exists.
    Array<Pointer<const Object> > Cycle(const L& Filter) const
    {
      Array<count> Path, DFS;
      Array<Pointer<const Object> > FoundCycle;

      Sortable::Array<Pointer<const Object> > Vertices = Nodes();

      Array<bool> Visited(Vertices.n()); Visited.Zero();
      Array<bool> InPath(Vertices.n()); InPath.Zero();
      Array<count> Predecessor(Vertices.n()); Predecessor.Zero();

      for(count i = 0; i < Vertices.n() and not FoundCycle.n(); i++)
      {
        Path.Clear();
        InPath.Zero();
        if(not Visited[i])
        {
          DFS.Push(i);
          while(DFS.n() and not FoundCycle.n())
          {
            count Vertex = DFS.Pop();
            if(not Visited[Vertex])
            {
              Visited[Vertex] = true;

              if(Path.n() and Path.z() != Predecessor[Vertex])
                InPath[Path.Pop()] = false;

              Path.Push(Vertex);
              InPath[Path.z()] = true;

              Array<Pointer<const Object> > Adjacencies =
                Vertices[Vertex]->Children(Filter);
              for(count j = 0; j < Adjacencies.n(); j++)
              {
                DFS.Push(Vertices.Search(Adjacencies[j]));
                Predecessor[DFS.z()] = Vertex;
              }
            }
            else if(InPath[Vertex])
              for(count j = Path.Search(Vertex); j < Path.n(); j++)
                FoundCycle.Push(Vertices[Path[j]]);
          }
        }
      }
      return FoundCycle;
    }

    //-------------//
    //String Output//
    //-------------//

    /**Returns a string version of the node or edge's label. If null is passed
    in then Null is returned. If the node or edge does not belong to the graph,
    then Alien is returned.*/
    String Print(Pointer<const Object> NodeOrEdge) const
    {
      if(not NodeOrEdge) return "Null";
      if(not Belongs(NodeOrEdge)) return "Alien";
      return *NodeOrEdge;
    }

    ///Returns a string version of an array of nodes and edges.
    String Print(Array<Pointer<Object> > NodesAndEdges) const
    {
      String s;
      s << "[";
      for(count i = 0; i < NodesAndEdges.n(); i++)
      {
        if(i) s << ", ";
        s << Print(NodesAndEdges[i]);
      }
      s << "]";
      return s;
    }

    ///Returns a string version of an array of nodes and edges.
    String Print(Array<Pointer<const Object> > NodesAndEdges) const
    {
      String s;
      s << "[";
      for(count i = 0; i < NodesAndEdges.n(); i++)
      {
        if(i) s << ", ";
        s << Print(NodesAndEdges[i]);
      }
      s << "]";
      return s;
    }

    ///Output a string version of the graph printing the nodes and edges.
    operator String() const
    {
      Array<Pointer<const Object> > NodeArray = Nodes();
      Array<Pointer<const Object> > EdgeArray = Edges();
      String s = "{{";
      for(count i = 0; i < NodeArray.n(); i++)
      {
        if(i) s << ", ";
        s << *NodeArray[i];
        if(NodeArray[i] == RootNode) s << "*";
      }
      s << "}, {";
      for(count i = 0; i < EdgeArray.n(); i++)
      {
        if(i) s << ", ";
        s << "{" << EdgeArray[i]->From->Label;
        s << "->" << EdgeArray[i]->To->Label;
        s << ", " << *EdgeArray[i] << "}";
      }
      s << "}}";
      return s;
    }

    ///Returns a representation of the graph in the TGF trivial graph format.
    String ExportTGF() const
    {
      Array<Pointer<const Object> > NodeArray = Nodes();
      Array<Pointer<const Object> > EdgeArray = Edges();

      String s;
      for(count i = 0; i < NodeArray.n(); i++)
        s >> i << " " << *NodeArray[i];

      s >> "#";

      for(count i = 0; i < EdgeArray.n(); i++)
        s >> NodeArray.Search(EdgeArray[i]->From) << " " <<
          NodeArray.Search(EdgeArray[i]->To) << " " << *EdgeArray[i];

      return s;
    }

    ///Returns a representation of the graph in the DOT graph format.
    String ExportDOT() const
    {
      Array<Pointer<const Object> > NodeArray = Nodes();
      Array<Pointer<const Object> > EdgeArray = Edges();

      String s;
      s >> "digraph g {";
      for(count i = 0; i < NodeArray.n(); i++)
      {
        String l = *NodeArray[i];
        l.Replace("\"", "\\\"");
        s >> "  " << i << " [label=\"" << l << "\"];";
      }
      for(count i = 0; i < EdgeArray.n(); i++)
      {
        String l = *EdgeArray[i];
        l.Replace("\"", "\\\"");
        s >> "  " << NodeArray.Search(EdgeArray[i]->From) << " -> " <<
          NodeArray.Search(EdgeArray[i]->To) << " [label=\"" << l << "\"];";
      }

      s >> "}";

      return s;
    }

    ///Returns a representation of the graph in the GraphML graph format.
    String ExportGraphML() const
    {
      String s;
      s >> "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
      s >> "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"  ";
      s >> "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
      s >> "    xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns";
      s >> "     http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">";
      s >> "  <key id=\"d0\" for=\"node\" attr.name=\"label\""
        " attr.type=\"string\">";
      s >> "    <default></default>";
      s >> "  </key>";
      s >> "  <key id=\"d1\" for=\"node\" attr.name=\"color\""
        " attr.type=\"string\">";
      s >> "    <default>#ffff00</default>";
      s >> "  </key>";

      s >> "  <graph id=\"G\" edgedefault=\"directed\">";

      Array<Pointer<const Object> > NodeArray = Nodes();
      Array<Pointer<const Object> > EdgeArray = Edges();

      for(count i = 0; i < NodeArray.n(); i++)
      {
        String LabelText = *NodeArray[i];
        LabelText.Replace("\"", "\\\"");
        String Color;
        if(LabelText.Contains("#"))
        {
          Color = LabelText;
          count Index = LabelText.Find("#");
          Color.Erase(0, Index - 1);
          LabelText.Erase(Index, LabelText.n());
        }
        s >> "    <node id=\"" << i << "\">";
        s >> "      <data key=\"d0\">" << LabelText << "</data>";
        if(Color)
          s >> "      <data key=\"d1\">" << Color << "</data>";
        s >> "    </node>";
      }
      for(count i = 0; i < EdgeArray.n(); i++)
      {
        String LabelText = *EdgeArray[i];
        LabelText.Replace("\"", "\\\"");
        s >> "    <edge source=\"" << NodeArray.Search(EdgeArray[i]->From) <<
          "\" target=\"" << NodeArray.Search(EdgeArray[i]->To) <<
          "\"/>";
      }

      s >> "  </graph>";
      s >> "</graphml>";
      s++;
      return s;
    }

    private:

    static String ExportXMLEscapeAttribute(String x)
    {
      x.Replace("\"", "&quot;");
      return x;
    }

    public:

    ///Returns an XML representation of the graph.
    String ExportXML(String RootTag = "graph") const
    {
      //Gather all the nodes in the graph.
      Array<Pointer<const Object> > NodeArray = Nodes();

      //Create the root tag.
      String s;
      s >> "<" << RootTag << ">";
      for(count i = 0; i < NodeArray.n(); i++)
      {
        //Create the node tag with its ID.
        Pointer<const Object> CurrentNode = NodeArray[i];
        s >> "  <node id=\"" << i << "\"";

        //Set the root attribute if the node is root.
        if(CurrentNode == Root())
          s << " root=\"root\"";

        //List the attributes.
        const L& NodeLabel = CurrentNode->Label;
        Array<String> NodeKeys = NodeLabel.AttributeKeysAsStrings();
        Array<String> NodeValues = NodeLabel.AttributeValuesAsStrings();
        for(count a = 0; a < NodeKeys.n(); a++)
          s << " " << NodeKeys[a] << "=\""
            << ExportXMLEscapeAttribute(NodeValues[a]) << "\"";

        //Gather a list of edges that use the current node as the departure.
        Array<Pointer<const Object> > DepartingEdges;
        typename Tree<Pointer<Object>, bool>::Iterator It;
        for(It.Begin(CurrentNode->Edges); It.Iterating(); It.Next())
        {
          Pointer<Object> Edge = It.Key();
          if(CurrentNode == Edge->From)
            DepartingEdges.Add() = Edge;
        }

        //If the node is a leaf then use the short form.
        s << (DepartingEdges.n() ? ">" : "/>");

        //Create edges as children of the node.
        for(count j = 0; j < DepartingEdges.n(); j++)
        {
          //Find the index of the 'to' node.
          s >> "    <edge to=\"" << NodeArray.Search(DepartingEdges[j]->To) <<
            "\"";

          //List the attributes of the edge.
          const L& EdgeLabel = DepartingEdges[j]->Label;
          Array<String> EdgeKeys = EdgeLabel.AttributeKeysAsStrings();
          Array<String> EdgeValues = EdgeLabel.AttributeValuesAsStrings();
          for(count a = 0; a < EdgeKeys.n(); a++)
            s << " " << EdgeKeys[a] << "=\"" <<
              ExportXMLEscapeAttribute(EdgeValues[a]) << "\"";
          s << "/>";
        }

        //Close tag if there were edges.
        if(DepartingEdges.n()) s >> "  </node>";
      }
      s >> "</" << RootTag << ">";
      s++;
      return s;
    }

    ///Clears the current graph and imports a graph exported with ExportXML.
    bool ImportXML(const String& XMLData, String RootTag = "graph")
    {
      Clear();
      //Read in the document.
      XML::Document d;

      //Attempt to read in the XML.
      if(XML::Parser::Error ParsingError = d.ParseDocument(XMLData))
      {
        C::Error() >> ParsingError.GetDescription();
        return false;
      }

      //Do sanity checks on the root node.
      if(not d.Root)
      {
        C::Error() >> "Error: no root node.";
        return false;
      }
      else if(d.Root->GetName() != RootTag)
      {
        C::Error() >> "Error: expected '" << RootTag << "' but '" <<
          d.Root->GetName() << "' is the root.";
        return false;
      }

      //Create a node tree.
      Tree<String, XML::Element*> NodeTree_;
      Tree<String, Pointer<Object> > GraphTree;

      //Make a node id tree.
      const List<XML::Object*> Objects = d.Root->GetObjects();
      for(count i = 0; i < Objects.n(); i++)
      {
        //Skip over any objects that are not elements.
        XML::Element* e = Objects[i]->IsElement();
        if(not e) continue;

        //Get the ID.
        String NodeID = e->GetAttributeValue("id");
        //Do some sanity checks on the incoming tag.
        if(e->GetName() != "node")
        {
          C::Error() >> "Error: Unexpected element '" << e->GetName() << "'.";
          Clear();
          return false;
        }
        else if(not NodeID)
        {
          C::Error() >> "Error: '" << e->GetName() << "' element with no id.";
          Clear();
          return false;
        }
        else if(NodeTree_[NodeID])
        {
          C::Error() >> "Error: Duplicate '" << e->GetName() << "' with id '" <<
            NodeID << "'";
          Clear();
          return false;
        }

        //Add the node element to the node tree and create a graph node.
        NodeTree_[NodeID] = e;
        GraphTree[NodeID] = Add();
      }

      //Create all the nodes in the graph, isolated at first.
      Tree<String, XML::Element*>::Iterator It;
      for(It.Begin(NodeTree_); It.Iterating(); It.Next())
      {
        //Parse attributes into node label.
        XML::Element* n = It.Value();
        Pointer<Object> From = GraphTree[n->GetAttributeValue("id")];
        const List<XML::Element::Attribute>& NodeAttributes =
          n->GetAttributes();
        for(count j = 0; j < NodeAttributes.n(); j++)
        {
          if(NodeAttributes[j].Name == "id") continue;
          if(NodeAttributes[j].Name == "root")
          {
            if(NodeAttributes[j].Value != "root")
            {
              C::Error() >>
                "Error: Expected value 'root' for attribute 'root'.";
              Clear();
              return false;
            }
            Root(From);
            continue;
          }
          From->Set(NodeAttributes[j].Name, NodeAttributes[j].Value);
        }

        //Recreate each edge in the node.
        const List<XML::Object*> EdgeList = n->GetObjects();
        for(count j = 0; j < EdgeList.n(); j++)
        {
          //Skip over any objects that are not elements.
          XML::Element* e = EdgeList[j]->IsElement();
          if(not e) continue;

          //Get the id of the to node.
          String ToID = e->GetAttributeValue("to");

          //Do some sanity checks on the incoming tag.
          if(e->GetName() != "edge")
          {
            C::Error() >> "Error: Unexpected element '" << e->GetName() << "'.";
            Clear();
            return false;
          }
          else if(not ToID)
          {
            C::Error() >> "Error: '" << e->GetName() << "' element with no id.";
            Clear();
            return false;
          }
          else if(not NodeTree_[ToID])
          {
            C::Error() >> "Error: no element found with id '" << ToID << "'.";
            Clear();
            return false;
          }

          //Connect the nodes together.
          Pointer<Object> NewEdge = Connect(From, GraphTree[ToID]);

          //Parse attributes into edge label.
          const List<XML::Element::Attribute>& EdgeAttributes =
            e->GetAttributes();
          for(count k = 0; k < EdgeAttributes.n(); k++)
          {
            if(EdgeAttributes[k].Name == "to") continue;
            NewEdge->Set(EdgeAttributes[k].Name, EdgeAttributes[k].Value);
          }
        }
      }
      return true;
    }

    /**Merges the nodes and edges from another graph into this one. Returns the
    root node of the incoming graph (which is no longer the root node). Note
    that the incoming graph will be empty at the end of this call.*/
    Pointer<Object> Merge(GraphT& Other)
    {
      typename Tree<Pointer<Object>, bool>::Iterator It;
      for(It.Begin(Other.NodeTree); It.Iterating(); It.Next())
        NodeTree[It.Key()] = It.Value();
      Other.NodeTree.RemoveAll();
      Pointer<Object> OtherRoot = Other.RootNode;
      Other.RootNode = Pointer<Object>::Weak();
      return OtherRoot;
    }

    /**Finds the shortest path from start to end nodes given the edge cost.
    Complexity is approximately O(n^2).*/
    List<Pointer<const Object> > ShortestPath(Pointer<const Object> Start,
      Pointer<const Object> End, const L& Filter) const
    {
      List<Pointer<const Object> > Result;
      if(Belongs(Start) and Belongs(End) and Start->IsNode() and End->IsNode())
      {
        Array<Pointer<const Object> > Vertices = Nodes();
        Tree<Pointer<const Object>, count> Indices;
        Array<number> Distances(Vertices.n());
        Array<count> Previous(Vertices.n());
        PriorityQueue<count, number> PriorityVertices;
        Array<bool> Scanned(Vertices.n());
        count EndIndex = 0;

        for(count i = 0; i < Vertices.n(); i++)
        {
          PriorityVertices.AddWithPriority(i, (Distances[i] =
            Vertices[i] == Start ? number(0) : Limits<number>::Infinity()));
          Indices[Vertices[i]] = i;
          Previous[i] = -1;
          Scanned[i] = false;
          if(Vertices[i] == End) EndIndex = i;
        }

        while(PriorityVertices.HasElements())
        {
          count u = PriorityVertices.ExtractMinimum();
          Scanned[u] = true;
          Array<Pointer<const Object> > Neighbors =
            Vertices[u]->Children(Filter, true);
          for(count i = 0; i < Neighbors.n(); i++)
          {
            count v = Indices[Neighbors[i]->Head()];
            if(not Scanned[v])
            {
              number Sum = Distances[u] + Neighbors[i]->Label.Cost();
              if(Sum < Distances[v])
              {
                Distances[v] = Sum;
                Previous[v] = u;
                PriorityVertices.ChangePriority(v, Sum);
              }
            }
          }
        }

        if(Previous[EndIndex] >= 0)
        {
          count Backtracker = EndIndex;
          while(Backtracker >= 0)
          {
            Result.Prepend(Vertices[Backtracker]),
            Backtracker = Previous[Backtracker];
          }
        }
      }
      return Result;
    }

    private:

    /**Contains a weak reference to the root of the graph. The root may be null
    if no root has been set or if the root node was deleted.*/
    typename Pointer<Object>::Weak RootNode;

    ///Tree of all the nodes in the graph.
    Tree<Pointer<Object>, bool> NodeTree;
  };

  /**A basic label container for a GraphT node or edge. As long as the below
  methods are implemented, the class can be used as the label type for the
  GraphT. The only basic requirement is that the types of gets and sets be
  symmetric, that is the type given is the type received. For this reason,
  trees are useful for storing label data.*/
  template <class T>
  class GraphTLabel
  {
    ///Stores a tree of data in the given type.
    Tree<T> Data;

    public:

    ///Const key-value lookup
    T Get(const T& Key) const {return Data[Key];}

    ///Mutable key-value lookup
    T& Set(const T& Key) {return Data[Key];}

    ///Attribute set for XML deserialization
    void Set(const ascii* Key, const ascii* Value) {Data[Key] = Value;}

    ///Returns attribute keys for the purpose of string serialization.
    Array<String> AttributeKeysAsStrings() const
    {
      Array<String> StringArray(Data.n());
      typename Tree<T>::Iterator It;
      count i = 0;
      for(It.Begin(Data); It.Iterating(); It.Next())
        StringArray[i++] = It.Key();
      return StringArray;
    }

    ///Returns attribute keys for the purpose of string serialization.
    Array<String> AttributeValuesAsStrings() const
    {
      Array<String> StringArray(Data.n());
      typename Tree<T>::Iterator It;
      count i = 0;
      for(It.Begin(Data); It.Iterating(); It.Next())
        StringArray[i++] = It.Value();
      return StringArray;
    }

    /**Indicates whether a given label is equivalent for edge traversal. The
    default behavior here is to traverse if the data is exactly the same.*/
    bool EdgeEquivalent(const GraphTLabel<T>& EdgeType) const
    {
      return Data == EdgeType.Data;
    }

    /**Indicates the cost of the label for shortest-path finding. The default
    is to treat all costs as the same.*/
    number Cost() const {return 1.0;}

    ///Converts the label to a string.
    operator String() const
    {
      return Data;
    }
  };
}
#endif
