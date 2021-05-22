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

#ifndef BELLE_ENGRAVER_GEOMETRY_H
#define BELLE_ENGRAVER_GEOMETRY_H

namespace BELLE_NAMESPACE
{
  ///Staff geometry detector to take graph of islands and parse it like a grid.
  class Geometry : public Value::Base
  {
    public: //methods

    ///Returns the number of parts detected.
    count GetNumberOfParts() const
    {
      return PartCount;
    }

    ///Returns the number of instants detected.
    count GetNumberOfInstants() const
    {
      return InstantCount;
    }

    ///Returns the instant range for a given part.
    VectorInt GetPartRange(count i) const
    {
      return PartInstantRange[i];
    }

    /**Goes through the island subgraph and determines its geometry. When debug
    mode is enabled, a print-out of transitive relationships is shown.*/
    bool Parse(const Music& mg)
    {
      Clear();
      GatherIslands(mg.Nodes());

      if(String Result = DetectLocalGraphErrors(mg))
      {
        C::Error() >> "Error: " << Result;
        Clear();
        return false;
      }

      if(not AssignPartIDs())
      {
        C::Error() >> "Error: The graph has conflicting island "
          "relationships such as crossing or self-intersecting parts.";
        Clear();
        return false;
      }

      if(not AssignInstantIDs(mg.Root()))
      {
        C::Error() >> "Error: The graph has conflicting island "
          "relationships such as crossing or self-intersecting instants.";
        Clear();
        return false;
      }

      AssignAccessors();

      for(count i = 0; i < GetNumberOfParts(); i++)
      {
        if(not LookupIsland(i, 0))
        {
          C::Error() >> "Error: The graph begins with fewer than the total "
            "number of parts in the graph. This is unsupported right now.";
          Clear();
          return false;
        }
      }

      return true;
    }

    ///Gets the part list for a given instant.
    void GetPartListForInstant(count InstantID, List<count>& PartList) const
    {
      PartList.RemoveAll();
      for(count i = 0; i < PartInstantRange.n(); i++)
        if(PartInstantRange[i].i() <= InstantID &&
          InstantID <= PartInstantRange[i].j())
            PartList.Add() = i;
    }

    ///Returns the top-most island in the instant.
    Music::ConstNode TopMostIslandInInstant(count InstantID) const
    {
      Music::ConstNode Isle;
      for(count i = 0; i < GetNumberOfParts(); i++)
        if((Isle = LookupIsland(i, InstantID)))
          break;
      return Isle;
    }

    ///Determines whether the instant is complete.
    bool IsInstantComplete(count InstantID) const
    {
      List<count> l;
      GetPartListForInstant(InstantID, l);
      return l.n() == PartsInInstant[InstantID];
    }

    ///Returns the beginning islands of each separate part.
    Array<Music::ConstNode> GetPartBeginnings() const
    {
      Array<Music::ConstNode> PartBeginnings;
      for(count i = 0; i < PartCount; i++)
        for(count j = 0; j < InstantCount; j++)
          if(Music::ConstNode x = LookupIsland(i, j))
            if(not x->Previous(MusicLabel(mica::Partwise)))
              PartBeginnings.Add() = x;
      return PartBeginnings;
    }

    ///Returns the number of parts detected for each instant.
    count GetPartsInInstant(count InstantID) const
    {
      return PartsInInstant[InstantID];
    }

    ///Looks up an island by part and instant IDs.
    Music::ConstNode LookupIsland(count PartID, count InstantID) const
    {
      if(PartID < 0 or PartID >= GetNumberOfParts() or InstantID < 0 or
        InstantID >= GetNumberOfInstants())
          return Music::ConstNode();
      else
        return IslandMatrix(PartID, InstantID);
    }

    ///Looks up an island by part and instant IDs.
    Music::ConstNode operator () (count PartID, count InstantID) const
    {
      return IslandMatrix(PartID, InstantID);
    }

    ///Returns a string describing the geometry.
    operator String () const
    {
      String s;

      //Display the extents.
      s >> "Part Count: " << GetNumberOfParts();
      s >> "" >> "Part Extents:";
      for(count i = 0; i < GetNumberOfParts(); i++)
        s >> "Part " << i << ": " << GetPartRange(i);

      s >> "" >> "Instant Count: " << GetNumberOfInstants();
      s >> "" >> "Instant Part Membership:";
      for(count i = 0; i < GetNumberOfInstants(); i++)
      {
        List<count> l;
        GetPartListForInstant(i, l);
        s >> "Instant " << i << ": " << l <<
          (IsInstantComplete(i) ? " (complete) " : " ");
      }
      return s;
    }

    ///Returns whether this geometry is equivalent to another.
    bool operator == (const Geometry& Other) const
    {
      //Number of parts must be the same.
      if(GetNumberOfParts() != Other.GetNumberOfParts())
        return false;

      //Number of instants must be the same.
      if(GetNumberOfInstants() != Other.GetNumberOfInstants())
        return false;

      //Part ranges for each part must be the same.
      for(count i = 0; i < GetNumberOfParts(); i++)
        if(GetPartRange(i) != Other.GetPartRange(i))
          return false;

      //Instants must be the same.
      for(count i = 0; i < GetNumberOfInstants(); i++)
      {
        //Instant complete flag must be the same.
        if(IsInstantComplete(i) != Other.IsInstantComplete(i))
          return false;

        //Part list for each instant must be the same.
        List<count> l, lOther;
        GetPartListForInstant(i, l);
        Other.GetPartListForInstant(i, lOther);

        if(l.n() != lOther.n())
          return false;

        for(count j = 0; j < l.n(); j++)
          if(l[j] != lOther[j])
            return false;
      }
      return true;
    }

    ///Returns whether this geometry is not equivalent to another.
    bool operator != (const Geometry& Other) const
    {
      return !(*this == Other);
    }

    ///Initialization constructor
    Geometry() : PartCount(0), InstantCount(0) {}

    ///Virtual destructor
    virtual ~Geometry();

    private: //members

    ///Contains subgraph of islands.
    Array<Music::ConstNode> Islands;

    ///Number of parts detected.
    count PartCount;

    ///Number of instants detected.
    count InstantCount;

    ///Contains bounds of parts as indexed into the Islands array.
    Array<Complex<Music::ConstNode> > PartBounds;

    ///Ranges of the parts in terms of their instant IDs.
    Array<VectorInt> PartInstantRange;

    ///Number of parts in each instant.
    Array<count> PartsInInstant;

    ///Accessor for island using instant by part.
    Matrix<Music::ConstNode> IslandMatrix;

    ///Clears the geometry information.
    void Clear()
    {
      Islands.Clear();
      PartCount = 0;
      InstantCount = 0;
      PartBounds.Clear();
      PartInstantRange.Clear();
      PartsInInstant.Clear();
      IslandMatrix.Clear();
    }

    private: //methods

    ///Look for simple local issues.
    String DetectLocalGraphErrors(const Music& mg)
    {
      //Detect issues with root node.
      if(not Islands.n() or not mg.Root())
        return "Graph is empty";
      if(mg.Root()->Get(mica::Type) != mica::Island)
        return "Root is not an island";
      if(mg.Root()->Previous(MusicLabel(mica::Partwise)))
        return "Root is not left-most island";
      if(mg.Root()->Previous(MusicLabel(mica::Instantwise)))
        return "Root is not top-most island";

      for(count i = 0; i < Islands.n(); i++)
      {
        if(Islands[i]->Children(MusicLabel(mica::Partwise)).n() > 1)
          return String("Island node " ) + String(Islands[i]) +
            " has more than one outgoing partwise edge";
        if(Islands[i]->Parents(MusicLabel(mica::Partwise)).n() > 1)
          return String("Island node " ) + String(Islands[i]) +
            " has more than one incoming partwise edge";
        if(Islands[i]->Children(MusicLabel(mica::Instantwise)).n() > 1)
          return String("Island node " ) + String(Islands[i]) +
            " has more than one outgoing instant-wise edge";
        if(Islands[i]->Parents(MusicLabel(mica::Instantwise)).n() > 1)
          return String("Island node " ) + String(Islands[i]) +
            " has more than one incoming instant-wise edge";
        if(not Islands[i]->Next(MusicLabel(mica::Partwise)) and
          not Islands[i]->Previous(MusicLabel(mica::Partwise)))
          return String("Island node " ) + String(Islands[i]) +
            " has neither incoming nor outgoing partwise edge (orphan)";
      }

      {
        Array<Music::ConstNode> Cycle = mg.Cycle(MusicLabel(mica::Partwise));
        if(Cycle.n())
          return String("Partwise cycle found: ") + mg.Print(Cycle) + ": " +
            Cycle;
      }

      {
        Array<Music::ConstNode> Cycle = mg.Cycle(MusicLabel(mica::Instantwise));
        if(Cycle.n())
          return String("Instant-wise cycle found: ") + mg.Print(Cycle) + ": " +
            Cycle;
      }

      Sortable::Array<Music::ConstNode> Nodes = mg.Nodes();
      for(count i = 0; i < Nodes.n(); i++)
      {
        Music::ConstNode n = Nodes[i];
        if(Music::ConstNode m = n->Next(MusicLabel(mica::Beam)))
        {
          if(n->Previous(MusicLabel(mica::Token)) ==
            m->Previous(MusicLabel(mica::Token)))
              return String("Found beaming within same island");
        }
      }

      return "";
    }

    ///Assigns accessors for reverse lookups.
    void AssignAccessors()
    {
      IslandMatrix.mn(PartCount, InstantCount);
      for(count i = 0; i < Islands.n(); i++)
      {
        count Instant = Islands[i]->Label.GetState("InstantID").AsCount();
        count Part = Islands[i]->Label.GetState("PartID").AsCount();
        IslandMatrix(Part, Instant) = Islands[i];
      }
    }

    /**Assigns part IDs to the island subgraph. They are assigned such that the
    minimum number of IDs are used and the part IDs ascend along instant-wise
    links. If the graph was created from an IslandGrid, then the pointer to that
    object can be passed in to help with debugging.*/
    bool AssignPartIDs()
    {
      //Mark the part strands.
      PartCount = MarkPartStrands();

      //Make sure the nodes connected instant-wise are partwise different.
      if(not CheckThatSeparatedPartsAreDifferent())
        return false;

      //Create the rule array.
      TransitiveMapping t(PartCount);

      //Observe all part relationships and store them in a rule matrix.
      ObservePartOrders(t);

      //Solve for the transitive closure and produce the new part mapping.
      t.Solve();

      //Check to see whether the graph is in a conflicted state.
      if(t.IsConflicted())
        return false;

      //Create part map from the transitive mapping.
      Array<count> PartMap;
      PartMap.n(PartCount);
      PartMap.Zero();
      for(count i = 0; i < PartCount; i++)
        PartMap[t.Mapping(i)] = i;

      //Go through each island and map its part.
      for(count i = 0; i < Islands.n(); i++)
        Islands[i]->Label.SetState("PartID") =
          PartMap[Islands[i]->Label.GetState("PartID").AsCount()];

      //Mark the part bounds.
      MarkPartBounds();

      return true;
    }

    ///Helper to convert array to list.
    template <class U> static void ArrayToList(Array<U>& a, List<U>& b)
    {
      b.RemoveAll();
      for(count i = 0; i < a.n(); i++)
        b.Add() = a[i];
    }

    ///Assigns ordered instant IDs using the leading edge algorithm.
    bool AssignInstantIDs(Music::ConstNode RootNode)
    {
      //Get the top.
      Music::ConstNode t = RootNode;
      Array<Music::ConstNode> FirstInstant;
      List<Music::ConstNode> LeadingEdge;

      Tree<Music::ConstNode, bool> Visited;

      //Gather the islands to the first instant.
      FirstInstant = t->Series(Music::Label(mica::Instantwise));
      ArrayToList(FirstInstant, LeadingEdge);

      //Define leading edge for the first instant.
      for(count i = 0; i < LeadingEdge.n(); i++)
        LeadingEdge[i]->Label.SetState("InstantID") = 0,
        Visited.Set(LeadingEdge[i]) = true;

      //Define part count for first instant.
      PartsInInstant.Clear();
      PartsInInstant.Add() = LeadingEdge.n();

      //Initialize the first non-initial instant ID.
      count InstantID = 1;

      //Iterate while there is a leading edge.
      while(LeadingEdge.n())
      {
        bool LeadingEdgeAdvanced = false;

        //Rotate through the leading edge looking for edges to push further.
        for(count i = 0; i < LeadingEdge.n(); i++)
        {
          //Get the next island.
          Music::ConstNode NextIsland = LeadingEdge[i]->Next(
            Music::Label(mica::Partwise));

          //If there is no next island, then remove this part entry.
          if(!NextIsland)
          {
            LeadingEdge.Remove(i--);
            LeadingEdgeAdvanced = true;
            continue;
          }

          //Get the instant group of the next island being tried.
          Array<Music::ConstNode> InstantGroup = NextIsland->Series(
            Music::Label(mica::Instantwise));

          //Find the penultimate group.
          Array<Music::ConstNode> PenultimateGroup;
          PenultimateGroup.n(InstantGroup.n());
          for(count j = 0; j < InstantGroup.n(); j++)
            PenultimateGroup[j] = InstantGroup[j]->Previous(
              Music::Label(mica::Partwise));

          //Determine if this group may advance the leading edge.
          bool GroupMayAdvance = true;
          for(count j = 0; j < PenultimateGroup.n(); j++)
          {
            //Skip parts that are being introduced.
            if(!PenultimateGroup[j])
              continue;

            //Look for an adjacency with the leading edge.
            bool FoundAdjacency = false;
            for(count k = 0; k < LeadingEdge.n(); k++)
            {
              if(PenultimateGroup[j] == LeadingEdge[k])
              {
                FoundAdjacency = true;
                break;
              }
            }

            //If no adjacency is found, the group may not advance the edge.
            if(!FoundAdjacency)
            {
              GroupMayAdvance = false;
              break;
            }
          }

          //The group can not advance, then continue the iteration.
          if(!GroupMayAdvance)
            continue;

          /*The group is advancing. Add any new parts to the leading edge. Note
          that part order dependence is not important for solving the leading
          edge, therefore the leading edge is simply appended to, rather than
          determining the proper location in which to insert the new part (which
          is possible but unnecessary). Also assign the instant IDs for this
          instant group.*/
          for(count j = 0; j < InstantGroup.n(); j++)
          {
            if(!PenultimateGroup[j])
            {
              LeadingEdge.Add() = InstantGroup[j];
              LeadingEdgeAdvanced = true;
            }
            else
            {
              //Find the adjacency and update the leading edge.
              for(count k = 0; k < LeadingEdge.n(); k++)
              {
                if(PenultimateGroup[j] == LeadingEdge[k])
                {
                  LeadingEdge[k] = InstantGroup[j];
                  LeadingEdgeAdvanced = true;
                  break;
                }
              }
            }

            InstantGroup[j]->Label.SetState("InstantID") = InstantID;
            if(Visited.Contains(InstantGroup[j]))
              return false;
            Visited.Set(InstantGroup[j]) = true;
          }

          //Record the number of parts detected in this instant.
          PartsInInstant.Add() = InstantGroup.n();

          //Increment the instant ID for the next leading edge determination.
          InstantID++;

          /*Since the group successfully advanced, try to continue along the
          same part (optional -- only affects internal ordering).*/
          i--;
        }

        if(not LeadingEdgeAdvanced)
          return false;
      }

      /*If not all the islands were visited, then there are orphaned parts of
      the geometry that could not be reached.*/
      if(Visited.n() != Islands.n())
        return false;

      //The instant ranges can now be marked.
      MarkInstantRanges();

      //Save the number of instants detected.
      InstantCount = InstantID;

      return true;
    }

    ///Gathers the island subgraph into an array of islands.
    void GatherIslands(const Sortable::Array<Music::ConstNode>& Nodes)
    {
      //Clear out the Islands array.
      Islands.Clear();

      //Gather all the islands together.
      for(count i = 0; i < Nodes.n(); i++)
        if(Nodes[i]->Get(mica::Type) == mica::Island)
          Islands.Add() = Nodes[i];
    }

    /**Takes a subgraph of island vertices and marks each part strand. Returns
    the number of parts detected.*/
    count MarkPartStrands()
    {
      //Mark each island with a part-index.
      count PartIndex = 0;
      for(count i = 0; i < Islands.n(); i++)
      {
        //Assumes that nodes exist and are valid islands.
        Music::ConstNode Current = Islands[i];

        //Skip over islands which are not the origin for their part.
        if(Current->Previous(Music::Label(mica::Partwise)))
            continue;

        //Tag all islands in a part strand with a part ID.
        while(Current)
        {
          Current->Label.SetState("PartID") = PartIndex;
          Current = Current->Next(Music::Label(mica::Partwise));
        }

        //Increment the part ID.
        PartIndex++;
      }

      //Return the number of parts.
      return PartIndex;
    }

    ///Checks that nodes connected instant-wise are from different parts.
    bool CheckThatSeparatedPartsAreDifferent() const
    {
      Music::ConstNode Next;
      bool FoundContradiction = false;
      for(count i = 0; i < Islands.n() and not FoundContradiction; i++)
        if((Next = Islands[i]->Next(Music::Label(mica::Instantwise))))
          if(Islands[i]->Label.GetState("PartID") ==
            Next->Label.GetState("PartID"))
              FoundContradiction = true;
      return not FoundContradiction;
    }

    ///Marks the bounds of each part.
    void MarkPartBounds()
    {
      //Size the parts bounds for the number of parts detected.
      PartBounds.n(PartCount);

      //Look for islands which start or end a part.
      for(count i = 0; i < Islands.n(); i++)
      {
        //Assumes that nodes exist and are valid islands.
        Music::ConstNode Current = Islands[i];

        //Get the current part ID.
        count PartID = Current->Label.GetState("PartID").AsCount();

        //Look for a start.
        if(!Current->Previous(Music::Label(mica::Partwise)))
          PartBounds[PartID].i() = Current;

        //Look for an end.
        if(!Current->Next(Music::Label(mica::Partwise)))
          PartBounds[PartID].j() = Current;
      }
    }

    ///Marks the instant ranges of each part.
    void MarkInstantRanges()
    {
      PartInstantRange.n(PartBounds.n());
      for(count i = 0; i < PartBounds.n(); i++)
      {
        PartInstantRange[i].i() =
          PartBounds[i].i()->Label.GetState("InstantID");
        PartInstantRange[i].j() =
          PartBounds[i].j()->Label.GetState("InstantID");
      }
    }

    ///Observes all part relationships and stores them in a transitive mapping.
    void ObservePartOrders(TransitiveMapping& t)
    {
      for(count i = 0; i < Islands.n(); i++)
      {
        Music::ConstNode Current = Islands[i];
        Music::ConstNode Next;
        if((Next = Current->Next(Music::Label(mica::Instantwise))))
          t.Set(Current->Label.GetState("PartID").AsCount(),
            Next->Label.GetState("PartID").AsCount(),
            TransitiveClosure::LessThan);
      }
    }
  };
}
#endif
