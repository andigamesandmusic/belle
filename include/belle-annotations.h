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

Array<Music::ConstNode> AnnotationsOfNode(Music::ConstNode MusicNode);
Music::Node AnnotationTreeOfSystem(Music& M);
Music::Node CreateAnnotation(Music& M, const Value& v);
void LinkAnnotation(Music& M, Music::Node Annotation, Music::Node MusicNode);
Array<Music::ConstNode> MusicNodesOfAnnotation(Music::ConstNode MusicNode);
Value PropertiesOfAnnotation(Music::ConstNode Annotation);

#ifdef BELLE_IMPLEMENTATION
Array<Music::ConstNode> AnnotationsOfNode(Music::ConstNode MusicNode)
{
  return MusicNode ? MusicNode->Parents(MusicLabel(mica::Annotation)) :
    Array<Music::ConstNode>();
}

Music::Node AnnotationTreeOfSystem(Music& M)
{
  Music::Node AnnotationTree;
  if(M.Root())
  {
    AnnotationTree = M.Root()->Next(MusicLabel(mica::AnnotationTree));
    if(not AnnotationTree)
    {
      AnnotationTree = M.Add();
      AnnotationTree->Set(mica::Type) = mica::AnnotationTree;
      M.Connect(M.Root(), AnnotationTree)->Set(mica::Type) =
        mica::AnnotationTree;
    }
  }
  return AnnotationTree;
}

Music::Node CreateAnnotation(Music& M, const Value& v)
{
  Music::Node Annotation;
  if(Music::Node AnnotationTree = AnnotationTreeOfSystem(M))
  {
    Annotation = M.Add();
    Annotation->Set(mica::Type) = mica::Annotation;
    Annotation->Set("Value") = JSON::Export(v);
    M.Connect(AnnotationTree, Annotation)->Set(mica::Type) = mica::Annotation;
  }
  return Annotation;
}

void LinkAnnotation(Music& M, Music::Node Annotation, Music::Node MusicNode)
{
  if(Music::Edge AnnotationLink = M.Connect(Annotation, MusicNode))
    AnnotationLink->Set(mica::Type) = mica::Annotation;
}

Array<Music::ConstNode> MusicNodesOfAnnotation(Music::ConstNode AnnotationNode)
{
  return AnnotationNode ?
    AnnotationNode->Children(MusicLabel(mica::Annotation)) :
    Array<Music::ConstNode>();
}

Value PropertiesOfAnnotation(Music::ConstNode Annotation)
{
  return Annotation ? JSON::Import(Annotation->Get("Value")) : Value();
}
#endif
