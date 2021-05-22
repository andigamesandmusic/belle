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

package MICA;

import java.util.UUID;
import java.util.HashMap;

class MICASequenceIndex {
    UUID _sequence;
    Integer _index;
    
    public MICASequenceIndex(UUID sequence, Integer index) {
        _sequence = sequence;
        _index = index;
    }
    
    public int hashCode() {
        return _sequence.hashCode() ^ _index;
    }
    
    public boolean equals(Object obj) {
        if(obj == null || !(obj instanceof MICASequenceIndex))
            return false;
        MICASequenceIndex rhs = (MICASequenceIndex)obj;
        return _sequence.equals(rhs._sequence) && _index.equals(rhs._index);
    }
}

class MICAConceptLanguage {
    UUID _concept;
    String _language;
    
    public MICAConceptLanguage(UUID concept, String language) {
        _concept = concept;
        _language = language;
    }
    
    public int hashCode() {
        return _concept.hashCode() ^ _language.hashCode();
    }
    
    public boolean equals(Object obj) {
        if(obj == null || !(obj instanceof MICAConceptLanguage))
            return false;
        MICAConceptLanguage rhs = (MICAConceptLanguage)obj;
        return _concept.equals(rhs._concept) && _language.equals(rhs._language);
    }
}

class MICASequenceConcept {
    UUID _sequence;
    UUID _concept;
    
    public MICASequenceConcept(UUID sequence, UUID concept) {
        _sequence = sequence;
        _concept = concept;
    }
    
    public int hashCode() {
        return _sequence.hashCode() ^ _concept.hashCode();
    }
    
    public boolean equals(Object obj) {
        if(obj == null || !(obj instanceof MICASequenceConcept))
            return false;
        MICASequenceConcept rhs = (MICASequenceConcept)obj;
        return _sequence.equals(rhs._sequence) && _concept.equals(rhs._concept);
    }
}

public class MICAData {
    /*
    The following hash-maps use a special naming convention:
    _keyType__valueType
    */
    
    static HashMap<String, UUID> _key__concept =
        new HashMap<String, UUID>();
        
    static HashMap<UUID, Integer> _concept__length =
        new HashMap<UUID, Integer>();

    static HashMap<MICASequenceConcept, Integer>
        _sequenceConcept__index =
        new HashMap<MICASequenceConcept, Integer>();
        
    static HashMap<MICASequenceIndex, UUID> _sequenceIndex__concept =
        new HashMap<MICASequenceIndex, UUID>();
        
    static HashMap<MICAConceptLanguage, String> _conceptLanguage__key =
        new HashMap<MICAConceptLanguage, String>();
        
    static HashMap<UUID, UUID> _hashedConcept__concept =
        new HashMap<UUID, UUID>();
        
    private static boolean initialized = false;
    
    private static MICAData initializer = new MICAData();
    
    private MICAData() {
        initialize();
    }
    
    static public void initialize() {
        System.out.println("Initializing MICA");
        //If data has already been initialized, do not reinitialize.
        if(initializer != null)
            initializer = null;
            
        if(initialized)
            return;
        else
            initialized = true;
        initializeAll();
    }
    
@@@@@2@@@@@
}
