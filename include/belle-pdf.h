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

#ifndef BELLE_PAINTERS_PDF_H
#define BELLE_PAINTERS_PDF_H

#ifdef BELLE_WITH_ZLIB
#include <zlib.h>
#endif

namespace BELLE_NAMESPACE
{
  /**Preliminary support for outputting Portable Document Files. The PDF class
  supports multiple pages and layers, and currently conforms to the
  PDF/X-1a:2001 standard (a subset of PDF tailored for reliable printing).*/
  class PDF : public Painter
  {
    public:

    class JPEGImage : public Image
    {
      friend class PDF;

      ///Stores an array of raw JPEG data.
      Array<byte> JPEGData;

      public:

      ///Loads an JPEG image from file.
      void Load(String JPEGFile)
      {
        File::Read(JPEGFile, JPEGData);
      }

      ///Loads a JPEG image from a raw byte array.
      void Load(const Array<byte>& JPEGData_)
      {
        JPEGData = JPEGData_;
      }

      ///Loads a JPEG image from a raw byte array.
      void Load(const byte* Data, count Bytes)
      {
        JPEGData.CopyMemoryFrom(Data, Bytes);
      }

      /**Creates a placeholder image given a resource ID. Use Load to import the
      image.*/
      JPEGImage(Resource& ResourceID) : Image(ResourceID) {}

      ///Creates an image given a resource ID and the filename of a JPEG.
      JPEGImage(Resource& ResourceID, String JPEGFile) :
        Image(ResourceID)
      {
        Load(JPEGFile);
      }

      ///Creates an image given a resource ID and raw JPEG data.
      JPEGImage(Resource& ResourceID, const Array<byte>& JPEGData_) :
        Image(ResourceID)
      {
        Load(JPEGData_);
      }

      ///Creates an image given a resource ID and raw JPEG data.
      JPEGImage(Resource& ResourceID, const byte* Data,
        count Bytes) : Image(ResourceID)
      {
        Load(Data, Bytes);
      }

      ///Virtual destructor.
      virtual ~JPEGImage();

      ///Returns the image size.
      VectorInt GetSize() const
      {
        const byte* ByteArray = &JPEGData.a();
        count Bytes = JPEGData.n();
        count i = 0;
        while(i < Bytes - 9)
        {
          byte Code = ByteArray[i]; i++;

          if(Code != 0xFF)
            return VectorInt();
          Code = ByteArray[i]; i++;
          switch(Code)
          {
            //Filler byte
            case 0xFF:
              i--;
              break;
            //Packets without data
            case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4:
            case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9:
              break;
            //Packets with size information
            case 0xC0: case 0xC1: case 0xC2: case 0xC3:
            case 0xC4: case 0xC5: case 0xC6: case 0xC7:
            case 0xC8: case 0xC9: case 0xCA: case 0xCB:
            case 0xCC: case 0xCD: case 0xCE: case 0xCF:
              i += 3;
              {
                int Height, Width;
                Height = int(ByteArray[i]) << 8 | int(ByteArray[i + 1]), i += 2;
                Width  = int(ByteArray[i]) << 8 | int(ByteArray[i + 1]);
                return VectorInt(Width, Height);
              }
            //Irrelevant variable-length packets
            default:
              int Length = int(ByteArray[i]) << 8 | int(ByteArray[i + 1]);
              i += 2 + Length - 2;
              break;
          }
        }
        return VectorInt();
      }
    };

    /**\brief Properties structure to supply the PDF class with additional
    PDF-specific information.*/
    class Properties : public Painter::Properties
    {
      public:
      /**A scaling value which is applied to all geometry. For Adobe's engine,
      this makes no difference because their curve generating algorithm depends
      on the resolution of the display it is drawing to. On other poorly
      designed renderers, the number of interpolations is proportional to the
      literal values of the numbers involved. This results in the renderer
      becoming to slow for very large numbers and too blocky for small numbers.
      The suggested value is 10000 as this will put it into the range of the
      PDFs they typically encounter.

      Update: it appears that software programs no longer are affected by the
      multiplier, and it is difficult to deal with the multiplier when images
      are involved due to the image space being constrained. It is recommended
      that this value remain at unit scale (keep at 1.0, the new default).*/
      number CTMMultiplier;
      String Filename;
      String Output;
      Array<byte> ExtraData;

      //Metadata
      String Title;
      String Author;

      Properties(String Filename_) : CTMMultiplier(1.0f),
        Filename(Filename_) {}
      Properties() : CTMMultiplier(1.0f) {}

      ///Virtual destructor
      virtual ~Properties();
    };

    private:
#ifdef BELLE_WITH_ZLIB
    static void GetZlibMetadata(String& PDFString, String Prefix,
      String& Out, count MaxBytesToAllocate = 50000000)
    {
      String Data;
      count i = 0;
      Out.Clear();

      for(;;)
      {
        String::Span j = PDFString.FindBetween(
          "/FlateDecode", "stream", Data, i);
        if(j == String::Span(-1, -1))
          break;

        j = PDFString.FindBetween("stream\n", "\nendstream", Data, i);
#if defined(JUCE_VERSION)
        {
          juce::MemoryInputStream In(Data.Merge(), (size_t)Data.n(), false);
          juce::GZIPDecompressorInputStream Deflater(In);
          Array<byte> InBuffer;
          InBuffer.n(10000);
          InBuffer.Zero();
          String Output;
          while(!Deflater.isExhausted())
          {
            count Actual =
              (count)Deflater.read(&InBuffer.a(), 10000);
            Output.Append(&InBuffer.a(), Actual);
          }
          Output.FindBetween(Prefix, ")", Out);
          if(Out.n())
            return;
        }
#else
        if(j.j() != -1)
        {
          count SizeTry = 100000; //Start with a modest assumption.
          while(SizeTry <= MaxBytesToAllocate)
          {
            //Allocate memory for uncompressed output (unknown size).
            uint8* OutputTry = new uint8[SizeTry];

            //Initialize Zlib structure.
            z_stream ZStream;
            Memory::Clear(ZStream);
            ZStream.avail_in = Data.n();
            ZStream.avail_out = SizeTry;
            ZStream.next_in = (Bytef*)Data.Merge();
            ZStream.next_out = (Bytef*)OutputTry;

            int rsti = inflateInit(&ZStream);
            if(rsti == Z_OK)
            {
              int rst2 = inflate(&ZStream, Z_FINISH);
              if(rst2 >= 0)
              {
                count OutputBytes = ZStream.total_out;
                String Output;
                Output.Append(OutputTry, OutputBytes);
                delete [] OutputTry;
                Output.FindBetween(Prefix, ")", Out);
                if(Out.n())
                  return;
                break;
              }
              else if(rst2 == Z_BUF_ERROR)
              {
                delete [] OutputTry;
                if(SizeTry < MaxBytesToAllocate)
                  SizeTry = Min(SizeTry * 2, MaxBytesToAllocate);
                else
                  SizeTry = MaxBytesToAllocate + 1;
              }
            }
            else
            {
              //There was a problem with this buffer.
              delete [] OutputTry;
              break;
            }
          }
        }
#endif
        i = j.j();
      }
    }
#endif

    public:

    ///Method to search an existing PDF file for created metadata.
    static void RetrievePDFMetadata(String Filename,
      Array<byte>& Metadata)
    {
      //Open up the file.
      String WholeFileString;
      File::Read(Filename, WholeFileString);
      String Code = "DF62391C36D34DFD83EE5B61177426FE ";
      String Result;
      WholeFileString.FindBetween(Code, ")", Result);

#ifdef BELLE_WITH_ZLIB
      if(!Result.n())
        GetZlibMetadata(WholeFileString, Code, Result);
#endif

      if(!Result.n())
      {
        Metadata.Clear();
        return;
      }

      count DataLength = Result.n() / 2;
      Metadata.n(DataLength);

      count j = 0;
      for(count i = 0; i < DataLength; i++)
      {
          byte b1 = Result[j];
          byte b2 = Result[j + 1];

        if(b1 >= byte('0') && b1 <= byte('9'))
          b1 -= byte('0');
        else if(b1 >= byte('A') && b1 <= byte('F'))
          b1 -= byte('A') - 10;
        else
        {
          Metadata.n(0);
          return;
        }

        if(b2 >= byte('0') && b2 <= byte('9'))
          b2 -= byte('0');
        else if(b2 >= byte('A') && b2 <= byte('F'))
          b2 -= byte('A') - 10;
        else
        {
          Metadata.n(0);
          return;
        }

        Metadata[i] = byte((b1 << 4) + b2);

        j += 2;
      }
    }

    ///Helper function to quickly get the metadata out of a PDF as a string.
    static void RetrievePDFMetadataAsString(String Filename,
      String& Metadata)
    {
      Array<byte> ByteData;
      RetrievePDFMetadata(Filename, ByteData);
      Metadata.Clear();
      if(ByteData.n())
        Metadata.Append(&ByteData.a(), ByteData.n());
    }

  protected:
    ///An internal representation of PDF objects
    struct Object
    {
      /**Stores the information for an embedded cross-reference. Whenever
      an object's stream cross-references another object, instead of
      immediately committing the reference, the insertion point and object
      pointer are saved so that after all the streams are committed, the
      insertions are made and the references are committed. This allows
      the objects to be instantiated and ordered arbitrarily.*/
      struct XRef
      {
        ///Pointer to the object that this XRef references
        Object* ObjectToReference;

        ///The point of insertion in the string
        count InsertionPoint;

        ///Default constructor zeroes the structure.
        XRef() : ObjectToReference(0), InsertionPoint(0) {}

        /**\brief This constructor allows the structure's data members
        to be passed as arguments.*/
        XRef(Object* ObjectToReference_, count InsertionPoint_)
        {
          ObjectToReference = ObjectToReference_;
          InsertionPoint = InsertionPoint_;
        }
      };

      ///This object's cross-reference index
      count XRefIndex;

      /**\brief This object's cross-reference offset from the beginning of
      the file*/
      count XRefOffset;

      /**\brief Indicates whether or not the content stream's double
      brackets should be automatically included.*/
      bool NoAutoBrackets; PRIM_PAD(bool)

      ///The object's PDF dictionary
      String Dictionary;

      ///The object's PDF content stream
      String Content;

      /**\brief A list of pending cross-references to be inserted into
      dictionaries*/
      List<XRef> DictionaryXRefs;

      /**\brief A list of pending cross-references to be inserted into
      content streams*/
      List<XRef> ContentXRefs;

      /**\brief Default constructor turns on auto-brackets and zeroes
      everything else.*/
      Object() : XRefIndex(0), XRefOffset(0), NoAutoBrackets(false) {}

      /**\brief Inserts an object cross-reference to be committed to the
      current end of the dictionary string.*/
      void InsertDictionaryXRef(Object* ObjectToReference)
      {
        XRef ObjectXRef(ObjectToReference, Dictionary.n());
        DictionaryXRefs.Append(ObjectXRef);
      }

      /**\brief Inserts an object cross-reference to be committed to the
      current end of the content stream string.*/
      void InsertContentXRef(Object* ObjectToReference)
      {
        XRef ObjectXRef(ObjectToReference, Content.n());
        ContentXRefs.Append(ObjectXRef);
      }

      ///Commits a list of cross-references to an object string.
      void CommitXRefList(List<XRef>& XRefList,
        String& ObjectString)
      {
        /**\brief Commit the cross-references by going through the list
        of XRefs and inserting the appropriate data into the string.
        \details Each insertion adds a few characters to the string, so
        this error must be corrected by keeping track of the insertion
        widths in InsertionBias.*/
        count InsertionBias = 0;
        for(count i = 0; i < XRefList.n(); i++)
        {
          XRef& CurrentXRef = XRefList[i];

          count IndexOfReferent =
            CurrentXRef.ObjectToReference->XRefIndex;
          count InsertionPointOfReferent =
            CurrentXRef.InsertionPoint;
          count FinalInsertionPoint =
            InsertionBias + InsertionPointOfReferent;

          String XRefString;
          XRefString << integer(IndexOfReferent);
          XRefString << " 0 R";

          ObjectString.Insert(XRefString, FinalInsertionPoint);

          InsertionBias += XRefString.n();
        }
      }
    };

    ///A list of the objects which compose the PDF file.
    List<Object*> Objects;

    /**A pointer to the currently active raster target. This object pointer
    is used by the drawing methods.*/
    Object* RasterObject;

    ///The number of images used.
    List<Object*> ImageList;
    Array<Resource> ImageResourceList;

    ///Cached pointer to the current portfolio being painted.
    Portfolio* CachedPortfolio;

    public:

    ///Default constructor for the PDF painter
    PDF() : RasterObject(0), CachedPortfolio(0) {}

    ///Virtual destructor
    virtual ~PDF();

    ///Properties of the PDF file
    PDF::Properties* PDFProperties;

    /**Internal method creates a new PDF object. PDF files are made of
    objects which are marked off by 1 0 obj and endobj. By storing these
    objects in a list, the actual indexing process can be deferred until the
    conclusion of data writing, so that the file can be optimally organized
    for debugging purposes.*/
    Object* CreatePDFObject()
    {
      Object* NewPDFObject = new Object;
      Objects.Append(NewPDFObject);
      return NewPDFObject;
    }

    /**\brief Writes all of the objects to a single flat stream as a string
    referenced in the parameter list.*/
    void CommitObjects(String& ByteStream)
    {
      //Put the objects in the same order they were created.
      for(count i = 0; i < Objects.n(); i++)
        Objects[i]->XRefIndex = i + 1;

      //The first object in this case will be the root.
      Object* RootObject = Objects.a();

      //The info (metadata) object will be the second object.
      Object* InfoObject = Objects[1];

      //Commit the indexes to each of the objects.
      for(count i = 0; i < Objects.n(); i++)
      {
        //Commit the dictionary cross-references.
        Objects[i]->CommitXRefList(
          Objects[i]->DictionaryXRefs,Objects[i]->Dictionary);

        //Commit the content cross-references.
        Objects[i]->CommitXRefList(
          Objects[i]->ContentXRefs,Objects[i]->Content);
      }

      //Write the header.
      ByteStream << '%' << "PDF-1.3"; //Adjust as necessary.
      ByteStream++;
      ByteStream << '%';
      ByteStream.Append(unicode(0xE2)); //a_Circumflex
      ByteStream.Append(unicode(0xE3)); //a_Tilde
      ByteStream.Append(unicode(0xCF)); //I_Umlaut
      ByteStream.Append(unicode(0xD3)); //O_Acute
      ByteStream++;

      //Write the objects.
      for(count XRefObject = 1; XRefObject < Objects.n() + 1;
        XRefObject++)
      {
        //Find the Object by looking it up by XRef object number.
        Object* CurrentObject = 0;
        for(count LookupIndex = 0; LookupIndex < Objects.n();
          LookupIndex++)
        {
          CurrentObject = Objects[LookupIndex];
          if(CurrentObject->XRefIndex == XRefObject)
            break;
        }

        /*Save the XRef offset of this object to help with the object
        table of contents at the end of the file.*/
        CurrentObject->XRefOffset = ByteStream.n();

        //Begin the object.
        ByteStream << XRefObject;
        ByteStream << " 0 obj";
        ByteStream++;

        //Decide whether or not to make brackets appear.
        if(!CurrentObject->NoAutoBrackets)
        {
          ByteStream << "<<";
          ByteStream++;
        }

        //Determine whether compression should be attempted.
        bool AttemptCompression = !CurrentObject->Dictionary.Contains("/Length")
          && CurrentObject->Content.n() > 0;

        //To turn off compression for debugging enable this line:
        //AttemptCompression = false;

        //Attempt to compress the stream.
        String CompressedStream;
        if(AttemptCompression)
          AttemptFlate(CurrentObject->Content, CompressedStream);

        //If the stream was compressed then add the entries to the dictionary.
        if(CompressedStream)
          CurrentObject->Dictionary >> "/Length " << CompressedStream.n()
            >> "/Filter /FlateDecode";
        else if(!CompressedStream && AttemptCompression)
          CurrentObject->Dictionary >> "/Length " << CurrentObject->Content.n();

        //Write the dictionary.
        ByteStream << CurrentObject->Dictionary;
        ByteStream++;

        //Close brackets if they were done before.
        if(!CurrentObject->NoAutoBrackets)
        {
          ByteStream << ">>";
          ByteStream++;
        }

        //If there is a content stream then write it.
        if(CurrentObject->Content.n() > 0)
        {
          ByteStream << "stream";
          ByteStream++;
          if(CompressedStream)
            ByteStream << CompressedStream;
          else
            ByteStream << CurrentObject->Content;
          ByteStream++;
          ByteStream << "endstream";
          ByteStream++;
        }

        //End the object.
        ByteStream << "endobj";
        ByteStream++;
        ByteStream++;
      }

      //Write the XRef table of contents found at the end of the PDF file.
      count XRefLocation = ByteStream.n();
      ByteStream << "xref";
      ByteStream >> "0 ";
      ByteStream << (Objects.n() + 1);
      ByteStream >> "0000000000 65535 f";
      ByteStream.Append(unicode(13));
      ByteStream.Append(unicode(10));

      //Write each XRef entry.
      for(count ObjectIndex = 1; ObjectIndex < Objects.n() + 1; ObjectIndex++)
      {
        //Again find the current object by looking it up by XRef index.
        Object* CurrentObject = 0;
        for(count LookupIndex = 0; LookupIndex < Objects.n(); LookupIndex++)
        {
          CurrentObject = Objects[LookupIndex];
          if(CurrentObject->XRefIndex == ObjectIndex)
            break;
        }

        //Was AppendInteger(CurrentObject->XRefOffset,10);
        String Integer; Integer << CurrentObject->XRefOffset;
        String PaddedInteger;
        for(count i = 0; i < 10 - Integer.n(); i++)
          PaddedInteger << "0";
        PaddedInteger << Integer;
        ByteStream << PaddedInteger;

        ByteStream << " 00000 n";
        /*PDF Reference states that "each line is 20 bytes long," which implies
        that the lines must terminate in CRLF.*/
        ByteStream.Append(unicode(13));
        ByteStream.Append(unicode(10));
      }

      //Create the file identifier.
      UUIDv4 FileID;
      String FileIDString;
      FileIDString << FileID;
      FileIDString.Replace("-", "");
      FileIDString = FileIDString.ToLower();

      //Write the PDF trailer.
      ByteStream << "trailer";
      ByteStream >> "<<";
      ByteStream >> "/Size ";
      ByteStream << (Objects.n() + 1);
      ByteStream >> "/Root ";
      ByteStream << RootObject->XRefIndex;
      ByteStream << " 0 R";
      ByteStream >> "/Info ";
      ByteStream << InfoObject->XRefIndex;
      ByteStream << " 0 R";
      ByteStream >> "/ID[<" << FileIDString << ">";
      ByteStream << "<" << FileIDString << ">]";
      ByteStream >> ">>";
      ByteStream >> "startxref";
      ByteStream >> XRefLocation;
      ByteStream >> '%' << '%' << "EOF";

      //Delete each object manually now that they are no longer necessary.
      for(count i = 0; i < Objects.n(); i++)
        delete Objects[i];

      //Remove everything in the list.
      Objects.RemoveAll();
    }

    ///Returns the current version of this painter.
    String GetProducer()
    {
      return "Belle PDF Painter";
    }

    String GetMetadata(Time CurrentTime, String Title,
      String Author)
    {
      String s;

      UUIDv4 XPacketID;
      String XPacketIDString;
      UUIDv4 DocumentID;
      String DocumentIDString;
      UUIDv4 InstanceID;
      String InstanceIDString;

      XPacketIDString << XPacketID;
      XPacketIDString.Replace("-", "");

      DocumentIDString << DocumentID;
      DocumentIDString = DocumentIDString.ToLower();

      InstanceIDString << InstanceID;
      InstanceIDString = InstanceIDString.ToLower();

      //A free XMP validator is here:
      //http://www.pdflib.com/knowledge-base/xmp-metadata/free-xmp-validator/

      s >> "<?xpacket begin=\"ï»¿\" id=\"" << XPacketIDString << "\"?>";
      s >> "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP "
        "Core 5.2-c001 63.139439, 2010/09/27-13:37:26        \">";
      s >> "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22"
        "-rdf-syntax-ns#\">";
      s >> "  <rdf:Description rdf:about=\"\"";
      s >> "    xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">";
      s >> "    <xmp:CreateDate>" << CurrentTime.ISO() << "</xmp:CreateDate>";
      s >> "    <xmp:ModifyDate>" << CurrentTime.ISO() << "</xmp:ModifyDate>";
      s >> "    <xmp:MetadataDate>" << CurrentTime.ISO() <<
        "</xmp:MetadataDate>";
      s >> "  </rdf:Description>";
      s >> "      <rdf:Description rdf:about=\"\"";
      s >> "            xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\">";
      s >> "         <pdf:Producer>" << GetProducer() << "</pdf:Producer>";
      s >> "      </rdf:Description>";
      s >> "      <rdf:Description rdf:about=\"\"";
      s >> "            xmlns:dc=\"http://purl.org/dc/elements/1.1/\">";
      s >> "         <dc:format>application/pdf</dc:format>";
      s >> "         <dc:title>";
      s >> "            <rdf:Alt>";
      s >> "               <rdf:li xml:lang=\"x-default\">" << Title <<
        "</rdf:li>";
      s >> "            </rdf:Alt>";
      s >> "         </dc:title>";
      s >> "         <dc:creator>";
      s >> "            <rdf:Seq>";
      s >> "               <rdf:li>" << Author << "</rdf:li>";
      s >> "            </rdf:Seq>";
      s >> "         </dc:creator>";
      s >> "      </rdf:Description>";
      s >> "      <rdf:Description rdf:about=\"\"";
      s >> "            xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\"";
      s >> "            xmlns:stEvt=\"http://ns.adobe.com/xap/1.0/sType/"
        "ResourceEvent#\">";
      s >> "         <xmpMM:DocumentID>uuid:" << DocumentIDString <<
        "</xmpMM:DocumentID>";
      s >> "         <xmpMM:InstanceID>uuid:" << InstanceIDString <<
        "</xmpMM:InstanceID>";
      s >> "         <xmpMM:RenditionClass>default</xmpMM:RenditionClass>";
      s >> "         <xmpMM:VersionID>1</xmpMM:VersionID>";
      s >> "      </rdf:Description>";
      s >> "      <rdf:Description rdf:about=\"\"";
      s >> "            xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">";
      s >> "         <pdfaid:part>1</pdfaid:part>";
      s >> "         <pdfaid:conformance>B</pdfaid:conformance>";
      s >> "      </rdf:Description>";
      s >> "</rdf:RDF>";
      s >> "</x:xmpmeta>";
      s >> "<?xpacket end=\"w\"?>";

      return s;
    }

    static String LiteralEscape(String s)
    {
      s.Replace("\n", "\\n");
      s.Replace("\r", "\\r");
      s.Replace("\t", "\\t");
      s.Replace("\b", "\\b");
      s.Replace("\f", "\\f");
      s.Replace("(", "\\(");
      s.Replace(")", "\\)");
      s.Replace("\\", "\\\\");
      return s;
    }

    //Attempts to compress the data using the flate algorithm.
    static void AttemptFlate(const String& In, String& Out)
    {
      //Clear the output.
      Out.Clear();

      //If there is no data to compress then just return.
      if(!In.n())
        return;

#if defined(JUCE_VERSION)
      //JUCE has a built-in GZIP compressor, so use that if it is available.
      juce::MemoryOutputStream CompressedOut;
      juce::GZIPCompressorOutputStream Compressor(&CompressedOut, 9, false);
      if(!Compressor.write(In.Merge(), size_t(In.n())))
        return;
      Compressor.flush();
      Out.Append((const byte*)CompressedOut.getData(),
        (count)CompressedOut.getDataSize());
#else
      //Fallback to uncompressed.
      return;
#endif
    }

    virtual void Paint(Portfolio* PortfolioToPaint,
      Painter::Properties* PortfolioProperties)
    {
      //Get the current time for embedding the dates in metadata.
      Time CurrentTime;

      //Ensure that we have a valid PDF::Properties object pointer.
      Properties* p = PortfolioProperties->Interface<Properties>();
      if(!p)
        return;
      p->Title = "";
      p->Author = "";

      //Remember the portfolio so that draw commands can access it later.
      CachedPortfolio = PortfolioToPaint;

      //Save for later reference by other methods.
      PDFProperties = p;

      //Create the main object entries in the PDF.
      Object* Catalog = CreatePDFObject(); //must be 1 0 R
      Object* Info = CreatePDFObject(); //must be 2 0 R
      Object* Metadata = CreatePDFObject();
      Object* Pages = CreatePDFObject();
      Object* FontCatalog = CreatePDFObject();
      Object* ImageCatalog = CreatePDFObject();
      Object* OutputIntent = CreatePDFObject();
      Object* DefaultFont = CreatePDFObject();
      Object* ExtraData = CreatePDFObject();
      Object* ICCProfile = 0;

      String ICCProfileString = ColorModels::sRGB::LookForProfile();
      if(ICCProfileString)
        ICCProfile = CreatePDFObject();

      //Create a default font.
      DefaultFont->Dictionary >> "/Type /Font" >> "/Subtype /Type1" >>
        "/BaseFont /Helvetica";

      FontCatalog->Dictionary >> " /DefaultFont ";
      FontCatalog->InsertDictionaryXRef(DefaultFont);

      /*Write the metadata. Note that according to the PDF/A standard, this
      section must be in plain text (not compressed). Therefore the length is
      specified to indicate to the object compressor to skip it.*/
      String XMPData = GetMetadata(CurrentTime, p->Title, p->Author);
      Metadata->Content << XMPData;
      Metadata->Dictionary >> "/Type /Metadata" >> "/Subtype /XML" >>
        "/Length " << Metadata->Content.n();

      //Set up the ICC profile.
      if(ICCProfile)
      {
        ICCProfile->Dictionary << "/N 3" >> "/Alternate /DeviceRGB";
        ICCProfile->Content << ICCProfileString;
      }

      //Create a catalog of the pages.
      Catalog->Dictionary = "/Type /Catalog";
      Catalog->Dictionary >> "/Pages ";
      Catalog->InsertDictionaryXRef(Pages);
      Catalog->Dictionary >> "/Metadata ";
      Catalog->InsertDictionaryXRef(Metadata);
      Catalog->Dictionary >> "/OutputIntents [ ";
      Catalog->InsertDictionaryXRef(OutputIntent);
      Catalog->Dictionary << " ]";

      //Grab the canvas list from the portfolio.
      List<Pointer<Canvas> >& cl  = PortfolioToPaint->Canvases;

      //An internal list of page content objects.
      List<Object*> PageObjects;

      //Loop through each canvas and commit it to a PDF page.
      for(count i = 0; i < cl.n(); i++)
      {
        //Create objects for page header and content information.
        Object* PageHeader = CreatePDFObject();
        Object* PageContent = RasterObject = CreatePDFObject();
        PageObjects.Append(PageHeader);

        Points Size = cl[i]->Dimensions;

        //Write the page's dictionary.
        PageHeader->Dictionary << "/Type /Page";
        PageHeader->Dictionary >> "/Parent ";
        PageHeader->InsertDictionaryXRef(Pages);
        PageHeader->Dictionary >> "/Contents ";
        PageHeader->InsertDictionaryXRef(PageContent);
        PageHeader->Dictionary >> "/MediaBox [ 0 0";
        PageHeader->Dictionary << " " << Size.x;
        PageHeader->Dictionary << " " << Size.y;
        PageHeader->Dictionary << " " << "]";

        PageHeader->Dictionary >> "/CropBox [ 0 0";
        PageHeader->Dictionary << " " << Size.x;
        PageHeader->Dictionary << " " << Size.y;
        PageHeader->Dictionary << " " << "]";

        PageHeader->Dictionary >> "/TrimBox [ 0 0";
        PageHeader->Dictionary << " " << Size.x;
        PageHeader->Dictionary << " " << Size.y;
        PageHeader->Dictionary << " " << "]";

        //Write out a reference to the catalog of fonts.
        PageHeader->Dictionary >> "/Resources";
        {
          PageHeader->Dictionary >> "  <<";
          PageHeader->Dictionary >> "    /Font ";
          PageHeader->InsertDictionaryXRef(FontCatalog);
          PageHeader->Dictionary >> "    /XObject ";
          PageHeader->InsertDictionaryXRef(ImageCatalog);
          if(ICCProfile)
          {
            PageHeader->Dictionary >> "    /ColorSpace << /ICCEmbeddedProfile ";
            PageHeader->Dictionary << "[/ICCBased ";
            PageHeader->InsertDictionaryXRef(ICCProfile);
            PageHeader->Dictionary << " " << "] >>";
          }
          PageHeader->Dictionary >> "  >>";
        }

        /*Convert device space into inches and divide by the
        CTMMultiplier, which allows applications which have static
        curve segmenting algorithms to produce smoother curves. For
        example FoxIt apparently uses the unit value as its step for
        segmentation meaning that if you are operating in inches then
        you have no chance of getting a smooth curve. Working in a
        "multiplied" CTM (in which the vectors themselves are multiplied
        by a number, allows the smoothing methods to work well on the
        unit assumption (which is not part of the PDF standard, and a
        poor algorithm, but it is a popular alternative viewer...)*/
        number CTMInches = 72.f / p->CTMMultiplier;
        PageContent->Content >> CTMInches;
        PageContent->Content << " " << "0 0";
        PageContent->Content << " " << CTMInches;
        PageContent->Content << " " << "0 0 cm";

        //Use RGB color which does not require conversion.
        if(ICCProfile)
        {
          PageContent->Content >> "/ICCEmbeddedProfile cs";
          PageContent->Content >> "/ICCEmbeddedProfile CS";
        }
        else
        {
          PageContent->Content >> "/DeviceRGB cs";
          PageContent->Content >> "/DeviceRGB CS";
        }

        //Save transformation matrix.
        PageContent->Content >> "q";

        //Set the page number.
        SetPageNumber(i);

        //Paint the main canvas layer.
        cl[i]->Paint(*this, *PortfolioToPaint);

        //Reset the page number to indicate painting is finished.
        ResetPageNumber();

        //Revert the transformation matrix.
        PageContent->Content >> "Q";

        //Set the current drawing target to null to be safe.
        RasterObject = 0;
      }

      //Write the table of contents for the pages.
      Pages->Dictionary = "/Type /Pages";
      Pages->Dictionary++;
      Pages->Dictionary << "/Kids [";
      for(count i = 0; i < PageObjects.n(); i++)
      {
        if(i != 0)
          Pages->Dictionary--;

        Pages->InsertDictionaryXRef(PageObjects[i]);
      }
      Pages->Dictionary << "]";
      Pages->Dictionary++;
      Pages->Dictionary << "/Count ";
      Pages->Dictionary << cl.n();

      //Create the catalog of images.
      for(count i = 0; i < ImageList.n(); i++)
      {
        ImageCatalog->Dictionary >> "/Im";
        ImageCatalog->Dictionary << i;
        ImageCatalog->Dictionary << " ";
        ImageCatalog->InsertDictionaryXRef(ImageList[i]);
      }

      //Create the info object.
      Info->Dictionary >> "/Title (" << LiteralEscape(p->Title) << ")";
      Info->Dictionary >> "/Author (" << LiteralEscape(p->Author) << ")";
      Info->Dictionary >> "/Producer (" << LiteralEscape(GetProducer()) << ")";
      Info->Dictionary >> "/CreationDate (D:" <<
        CurrentTime.ISOCondensed() << ")";
      Info->Dictionary >> "/ModDate (D:" << CurrentTime.ISOCondensed() << ")";

      //Create the output intent for PDF-A1 compliance.
      if(ICCProfile)
      {
        OutputIntent->Dictionary >> "/Type /OutputIntent";
        OutputIntent->Dictionary >> "/DestOutputProfile ";
        OutputIntent->InsertDictionaryXRef(ICCProfile);
        OutputIntent->Dictionary >> "/Info (sRGB IEC61966-2.1)";
        OutputIntent->Dictionary >> "/OutputCondition ()";
        OutputIntent->Dictionary >> "/OutputConditionIdentifier (Custom)";
        OutputIntent->Dictionary >> "/RegistryName (http://www.color.org)";
        OutputIntent->Dictionary >> "/S /GTS_PDFA1";
      }

      /*Metadata is saved as the contents of a ghost page which is never
      referenced and tagged with a special UUIDv4 tag. The information is stored
      in hex (which gets compressed).*/
      {
        String MetadataPayload;
        MetadataPayload >> "BT /DefaultFont 1 Tf 0 0 Td 3 Tr (";

        /*Write the tag that contains a UUIDv4 identifying to a data importer
        that this section unambiguously contains info encoded in hex code.*/
        MetadataPayload << "DF62391C36D34DFD83EE5B61177426FE ";

        //Write the data as hex in case some of it is binary.
        Array<byte> HexByteString;
        p->ExtraData.EncodeAsASCIIHex(HexByteString);
        MetadataPayload.Append(&HexByteString[0], HexByteString.n());
        MetadataPayload << ") Tj ET";

        ExtraData->Content >> MetadataPayload;
      }

      //Commit all of the objects to the output string.
      CommitObjects(p->Output);

      //If applicable send the output to file.
      if(p->Filename != "")
        File::Write(p->Filename.Merge(), p->Output);

      //Clear the cached portfolio.
      CachedPortfolio = 0;
    }

    //---------------//
    //Drawing Methods//
    //---------------//

  protected:

    ///Checks the number and throws an error if it is problematic.
    number Check(number x)
    {
#ifdef BELLE_DEBUG_OBSERVE_PROBLEMATIC_NUMBERS_DURING_PAINTING
      const number Acceptable = 1.0e+6;
      if(not (x > -Acceptable and x < Acceptable))
      {
        C::Error() >> "PDF Painter: bad number detected: " << x;
        int y = 1; C::Out() >> "Halting..."; C::Out() << int(x) / (y ^ y);
      }
#endif
      return x;
    }

    void Rasterize(String t)
    {
      if(RasterObject)
        RasterObject->Content >> t;
    }

    //---------------------//
    //Affine Transformation//
    //---------------------//

    virtual void Transform(const Affine& a)
    {
      //Call the base class transform first.
      Painter::Transform(a);

      //Create the transform code.
      number CTMMultiplier = PDFProperties->CTMMultiplier;
      String t;
      t >> "q" >> Check(a.a) << " " << Check(a.b) << " " << Check(a.c) << " " <<
        Check(a.d) << " " << Check(a.e * CTMMultiplier) << " " <<
        Check(a.f * CTMMultiplier) << " cm";

      //Write it to the PDF.
      Rasterize(t);
    }

    virtual void Revert(count TransformationsToRevert)
    {
      //Call the base class revert first.
      Painter::Revert(TransformationsToRevert);

      //Create the revert code.
      String t;
      for(count i = 0; i < TransformationsToRevert; i++)
        t >> "Q";
      Rasterize(t);

      /*PDF includes fill and stroke color in the graphics stack, so to continue
      with the same raster state, it needs to be set again so that behavior will
      be predictable.*/
      SetBrush(State);
    }

    virtual void SetBrush(const Brush& NewState)
    {
      State = NewState;

      String t;

      t >> Check(NewState.StrokeColor.R);
      t << " " << Check(NewState.StrokeColor.G);
      t << " " << Check(NewState.StrokeColor.B);
      t << " " << "SC";

      t >> Check(NewState.FillColor.R);
      t << " " << Check(NewState.FillColor.G);
      t << " " << Check(NewState.FillColor.B);
      t << " " << "sc";

      Rasterize(t);
    }

    //-----//
    //Paths//
    //-----//

    virtual void Draw(const Path& p, const Affine& a)
    {
      String t;
      number CTMMultiplier = PDFProperties->CTMMultiplier;

      if(State.StrokeWidth != 0.f)
      {
        t >> Check(Abs(State.StrokeWidth) * CTMMultiplier);
        t << " " << "w";
      }

      for(count j = 0; j < p.n(); j++)
      {
        const Instruction& i = p[j];

        if(i.IsMove())
        {
          t >> Check(i.End().x * CTMMultiplier);
          t << " " << Check(i.End().y * CTMMultiplier);
          t << " " << "m";
        }
        else if(i.IsLine())
        {
          t >> Check(i.End().x * CTMMultiplier);
          t << " " << Check(i.End().y * CTMMultiplier);
          t << " " << "l";
        }
        else if(i.IsCubic())
        {
          t >> Check(i.Control1().x * CTMMultiplier);
          t << " " << Check(i.Control1().y * CTMMultiplier);
          t << " " << Check(i.Control2().x * CTMMultiplier);
          t << " " << Check(i.Control2().y * CTMMultiplier);
          t << " " << Check(i.End().x * CTMMultiplier);
          t << " " << Check(i.End().y * CTMMultiplier);
          t << " " << "c";
        }
        else if(i.IsClosing())
        {
          t << " " << "h";
        }
      }

      if(State.StrokeWidth > 0.f && State.StrokeColor.A > 0.f &&
        State.FillColor.A == 0.f)
          t >> "S"; //Stroke only.
      else if((State.StrokeWidth == 0.f || State.StrokeColor.A == 0.f) &&
        State.FillColor.A >= 0.f)
          t >> "f"; //Fill only.
      else if(State.StrokeWidth > 0.f && State.StrokeColor.A > 0.f &&
        State.FillColor.A >= 0.f)
          t >> "B"; //Fill and stroke.
      else
        t >> "n"; //"No-op"

      //Collapse the temporary operator string.
      t.Merge();

      Transform(a);
      Rasterize(t);
      Revert(1);
    }

    virtual void Draw(const Resource& ResourceID, Vector Size)
    {
      //Need access to the portfolio to get access to the resources.
      if(!CachedPortfolio) return;

      //Attempt to load the resource.
      const JPEGImage* ImageResource =
        CachedPortfolio->FindImage<JPEGImage>(ResourceID);
      if(!ImageResource) return;

      //If the data contains nothing do not continue.
      if(!ImageResource->JPEGData.n()) return;

      //Get the dimensions.
      VectorInt Pixels = ImageResource->GetSize();
      count PixelsWidth = count(Pixels.x), PixelsHeight = count(Pixels.y);
      number Width = Size.x, Height = Size.y;

      //Determine the aspect ratio.
      number AspectRatio = number(PixelsHeight) / number(PixelsWidth);
      if(not PixelsWidth and PixelsHeight)
        Width = Height / AspectRatio;
      else if(not PixelsHeight and PixelsWidth)
        Height = Width * AspectRatio;
      else if(not PixelsWidth and not PixelsHeight)
        return;

      //Get image data.
      String ImageString(&ImageResource->JPEGData.a(),
        ImageResource->JPEGData.n());

      //Attempt to find the resource first.
      count ImageResourceIndex = ImageResourceList.Search(*ImageResource);

      //If it does not exist yet, then create an entry for it.
      if(ImageResourceIndex == -1)
      {
        //Create an image object (an XObject in the PDF file).
        ImageList.Add() = CreatePDFObject();
        ImageResourceList.Add() = *ImageResource;

        //Enter in the appropriate dictionary information.
        String Dictionary;
        Dictionary >> "   /Type /XObject";
        Dictionary >> "   /Subtype /Image";
        Dictionary >> "   /Width "  << PixelsWidth;
        Dictionary >> "   /Height " << PixelsHeight;
        Dictionary >> "   /ColorSpace /DeviceRGB"; //RGB only for now.
        Dictionary >> "   /BitsPerComponent 8"; //Always 8 for JPEGs.
        Dictionary >> "   /Length " << ImageString.n();
        Dictionary >> "   /Filter /DCTDecode";
        ImageList.z()->Dictionary = Dictionary;

        /*Load the data into the stream, filtered by hex to make things easier.
        Note that because of the hex filter, PDF has placed a restriction that
        the JPEG can not be of the progressive format. For now we will just
        assume that the JPEG is not in this format.*/
        ImageList.z()->Content = ImageString;

        //Get the index of the image that was added.
        ImageResourceIndex = ImageResourceList.n() - 1;
      }

      /*Add the image painting operator. Note that image space is defined by the
      PDF specification to be from [0, 0] to [1, 1]. Thus the proper common
      transformation matrix must be used for the image to scale correctly.*/
      String t;
      Scale(Vector(Width, Height));
      t << "/Im" << ImageResourceIndex << " Do";
      Rasterize(t);
      Revert(1);
    }
  };
}
#endif
