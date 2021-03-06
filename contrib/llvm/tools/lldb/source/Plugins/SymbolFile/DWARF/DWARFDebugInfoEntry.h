//===-- DWARFDebugInfoEntry.h -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SymbolFileDWARF_DWARFDebugInfoEntry_h_
#define SymbolFileDWARF_DWARFDebugInfoEntry_h_

#include "SymbolFileDWARF.h"
#include "llvm/ADT/SmallVector.h"

#include "DWARFDebugAbbrev.h"
#include "DWARFAbbreviationDeclaration.h"
#include "DWARFDebugRanges.h"
#include <vector>
#include <map>
#include <set>

typedef std::map<const DWARFDebugInfoEntry*, dw_addr_t>     DIEToAddressMap;
typedef DIEToAddressMap::iterator                           DIEToAddressMapIter;
typedef DIEToAddressMap::const_iterator                     DIEToAddressMapConstIter;

typedef std::map<dw_addr_t, const DWARFDebugInfoEntry*>     AddressToDIEMap;
typedef AddressToDIEMap::iterator                           AddressToDIEMapIter;
typedef AddressToDIEMap::const_iterator                     AddressToDIEMapConstIter;


typedef std::map<dw_offset_t, dw_offset_t>                  DIEToDIEMap;
typedef DIEToDIEMap::iterator                               DIEToDIEMapIter;
typedef DIEToDIEMap::const_iterator                         DIEToDIEMapConstIter;

typedef std::map<uint32_t, const DWARFDebugInfoEntry*>      UInt32ToDIEMap;
typedef UInt32ToDIEMap::iterator                            UInt32ToDIEMapIter;
typedef UInt32ToDIEMap::const_iterator                      UInt32ToDIEMapConstIter;

typedef std::multimap<uint32_t, const DWARFDebugInfoEntry*> UInt32ToDIEMMap;
typedef UInt32ToDIEMMap::iterator                           UInt32ToDIEMMapIter;
typedef UInt32ToDIEMMap::const_iterator                     UInt32ToDIEMMapConstIter;

class DWARFDeclContext;

#define DIE_SIBLING_IDX_BITSIZE 31
#define DIE_ABBR_IDX_BITSIZE 15

class DWARFDebugInfoEntry
{
public:
    typedef std::vector<DWARFDebugInfoEntry>    collection;
    typedef collection::iterator                iterator;
    typedef collection::const_iterator          const_iterator;

    typedef std::vector<dw_offset_t>            offset_collection;
    typedef offset_collection::iterator         offset_collection_iterator;
    typedef offset_collection::const_iterator   offset_collection_const_iterator;

    class Attributes
    {
    public:
        Attributes();
        ~Attributes();

        void Append(const DWARFCompileUnit *cu, dw_offset_t attr_die_offset, dw_attr_t attr, dw_form_t form);
        const DWARFCompileUnit * CompileUnitAtIndex(uint32_t i) const { return m_infos[i].cu; }
        dw_offset_t DIEOffsetAtIndex(uint32_t i) const { return m_infos[i].die_offset; }
        dw_attr_t AttributeAtIndex(uint32_t i) const { return m_infos[i].attr; }
        dw_attr_t FormAtIndex(uint32_t i) const { return m_infos[i].form; }
        bool ExtractFormValueAtIndex (SymbolFileDWARF* dwarf2Data, uint32_t i, DWARFFormValue &form_value) const;
        uint64_t FormValueAsUnsignedAtIndex (SymbolFileDWARF* dwarf2Data, uint32_t i, uint64_t fail_value) const;
        uint64_t FormValueAsUnsigned (SymbolFileDWARF* dwarf2Data, dw_attr_t attr, uint64_t fail_value) const;
        uint32_t FindAttributeIndex(dw_attr_t attr) const;
        bool ContainsAttribute(dw_attr_t attr) const;
        bool RemoveAttribute(dw_attr_t attr);
        void Clear() { m_infos.clear(); }
        size_t Size() const { return m_infos.size(); }

    protected:
        struct Info
        {
            const DWARFCompileUnit *cu; // Keep the compile unit with each attribute in case we have DW_FORM_ref_addr values
            dw_offset_t die_offset;
            dw_attr_t attr;
            dw_form_t form;
        };

        typedef llvm::SmallVector<Info, 32> collection;
        collection m_infos;
    };

    struct CompareState
    {
        CompareState() :
            die_offset_pairs()
        {
            assert(sizeof(dw_offset_t)*2 == sizeof(uint64_t));
        }

        bool AddTypePair(dw_offset_t a, dw_offset_t b)
        {
            uint64_t a_b_offsets = (uint64_t)a << 32 | (uint64_t)b;
            // Return true if this type was inserted, false otherwise
            return die_offset_pairs.insert(a_b_offsets).second;
        }
        std::set< uint64_t > die_offset_pairs;
    };

                DWARFDebugInfoEntry():
                    m_offset        (DW_INVALID_OFFSET),
                    m_parent_idx    (0),
                    m_sibling_idx   (0),
                    m_empty_children(false),
                    m_abbr_idx      (0),
                    m_has_children  (false),
                    m_tag           (0)
                {
                }

    void        Clear ()
                {
                    m_offset         = DW_INVALID_OFFSET;
                    m_parent_idx     = 0;
                    m_sibling_idx    = 0;
                    m_empty_children = false;
                    m_abbr_idx       = 0;
                    m_has_children   = false;
                    m_tag            = 0;
                }

    bool        Contains (const DWARFDebugInfoEntry *die) const;

    void        BuildAddressRangeTable(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    DWARFDebugAranges* debug_aranges) const;

    void        BuildFunctionAddressRangeTable(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    DWARFDebugAranges* debug_aranges) const;

    bool        FastExtract(
                    const lldb_private::DataExtractor& debug_info_data,
                    const DWARFCompileUnit* cu,
                    const uint8_t *fixed_form_sizes,
                    lldb::offset_t* offset_ptr);

    bool        Extract(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    lldb::offset_t* offset_ptr);

    bool        LookupAddress(
                    const dw_addr_t address,
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    DWARFDebugInfoEntry** function_die,
                    DWARFDebugInfoEntry** block_die);

    size_t      GetAttributes(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const uint8_t *fixed_form_sizes,
                    DWARFDebugInfoEntry::Attributes& attrs,
                    uint32_t curr_depth = 0) const; // "curr_depth" for internal use only, don't set this yourself!!!

    dw_offset_t GetAttributeValue(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    DWARFFormValue& formValue,
                    dw_offset_t* end_attr_offset_ptr = NULL) const;

    const char* GetAttributeValueAsString(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    const char* fail_value) const;

    uint64_t    GetAttributeValueAsUnsigned(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    uint64_t fail_value) const;

    uint64_t    GetAttributeValueAsReference(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    uint64_t fail_value) const;

    int64_t     GetAttributeValueAsSigned(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    int64_t fail_value) const;

    dw_addr_t   GetAttributeHighPC(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    dw_addr_t lo_pc,
                    uint64_t fail_value) const;

    bool        GetAttributeAddressRange(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    dw_addr_t& lo_pc,
                    dw_addr_t& hi_pc,
                    uint64_t fail_value) const;

    dw_offset_t GetAttributeValueAsLocation(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_attr_t attr,
                    lldb_private::DataExtractor& data,
                    uint32_t &block_size) const;

    const char* GetName(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu) const;

    const char* GetMangledName(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    bool substitute_name_allowed = true) const;

    const char* GetPubname(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu) const;

    static bool GetName(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_offset_t die_offset,
                    lldb_private::Stream &s);

    static bool AppendTypeName(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const dw_offset_t die_offset,
                    lldb_private::Stream &s);

    const char * GetQualifiedName (
                    SymbolFileDWARF* dwarf2Data, 
                    DWARFCompileUnit* cu,
                    std::string &storage) const;
    
    const char * GetQualifiedName (
                    SymbolFileDWARF* dwarf2Data, 
                    DWARFCompileUnit* cu,
                    const DWARFDebugInfoEntry::Attributes& attributes,
                    std::string &storage) const;

//    static int  Compare(
//                    SymbolFileDWARF* dwarf2Data,
//                    dw_offset_t a_die_offset,
//                    dw_offset_t b_die_offset,
//                    CompareState &compare_state,
//                    bool compare_siblings,
//                    bool compare_children);
//
//    static int Compare(
//                    SymbolFileDWARF* dwarf2Data,
//                    DWARFCompileUnit* a_cu, const DWARFDebugInfoEntry* a_die,
//                    DWARFCompileUnit* b_cu, const DWARFDebugInfoEntry* b_die,
//                    CompareState &compare_state,
//                    bool compare_siblings,
//                    bool compare_children);

    static bool OffsetLessThan (
                    const DWARFDebugInfoEntry& a,
                    const DWARFDebugInfoEntry& b);

    void        Dump(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    lldb_private::Stream &s,
                    uint32_t recurse_depth) const;

    void        DumpAncestry(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const DWARFDebugInfoEntry* oldest,
                    lldb_private::Stream &s,
                    uint32_t recurse_depth) const;

    static void DumpAttribute(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const lldb_private::DataExtractor& debug_info_data,
                    lldb::offset_t *offset_ptr,
                    lldb_private::Stream &s,
                    dw_attr_t attr,
                    dw_form_t form);
    // This one dumps the comp unit name, objfile name and die offset for this die so the stream S.
    void          DumpLocation(
                    SymbolFileDWARF* dwarf2Data,
                    DWARFCompileUnit* cu,
                    lldb_private::Stream &s) const;
                    
    bool        GetDIENamesAndRanges(
                    SymbolFileDWARF* dwarf2Data,
                    const DWARFCompileUnit* cu,
                    const char * &name,
                    const char * &mangled,
                    DWARFDebugRanges::RangeList& rangeList,
                    int& decl_file,
                    int& decl_line,
                    int& decl_column,
                    int& call_file,
                    int& call_line,
                    int& call_column,
                    lldb_private::DWARFExpression *frame_base = NULL) const;

    const DWARFAbbreviationDeclaration* 
    GetAbbreviationDeclarationPtr (SymbolFileDWARF* dwarf2Data,
                                   const DWARFCompileUnit *cu,
                                   lldb::offset_t &offset) const;

    dw_tag_t
    Tag () const 
    {
        return m_tag;
    }

    bool
    IsNULL() const 
    {
        return m_abbr_idx == 0; 
    }

    dw_offset_t
    GetOffset () const 
    { 
        return m_offset; 
    }

    void
    SetOffset (dw_offset_t offset)
    { 
        m_offset = offset; 
    }

    bool
    HasChildren () const 
    { 
        return m_has_children;
    }
    
    void
    SetHasChildren (bool b)
    {
        m_has_children = b;
    }

            // We know we are kept in a vector of contiguous entries, so we know
            // our parent will be some index behind "this".
            DWARFDebugInfoEntry*    GetParent()             { return m_parent_idx > 0 ? this - m_parent_idx : NULL;  }
    const   DWARFDebugInfoEntry*    GetParent()     const   { return m_parent_idx > 0 ? this - m_parent_idx : NULL;  }
            // We know we are kept in a vector of contiguous entries, so we know
            // our sibling will be some index after "this".
            DWARFDebugInfoEntry*    GetSibling()            { return m_sibling_idx > 0 ? this + m_sibling_idx : NULL;  }
    const   DWARFDebugInfoEntry*    GetSibling()    const   { return m_sibling_idx > 0 ? this + m_sibling_idx : NULL;  }
            // We know we are kept in a vector of contiguous entries, so we know
            // we don't need to store our child pointer, if we have a child it will
            // be the next entry in the list...
            DWARFDebugInfoEntry*    GetFirstChild()         { return (HasChildren() && !m_empty_children) ? this + 1 : NULL; }
    const   DWARFDebugInfoEntry*    GetFirstChild() const   { return (HasChildren() && !m_empty_children) ? this + 1 : NULL; }

    
    void                            GetDeclContextDIEs (SymbolFileDWARF* dwarf2Data, 
                                                        DWARFCompileUnit* cu,
                                                        DWARFDIECollection &decl_context_dies) const;

    void                            GetDWARFDeclContext (SymbolFileDWARF* dwarf2Data,
                                                         DWARFCompileUnit* cu,
                                                         DWARFDeclContext &dwarf_decl_ctx) const;


    bool                            MatchesDWARFDeclContext(SymbolFileDWARF* dwarf2Data,
                                                            DWARFCompileUnit* cu,
                                                            const DWARFDeclContext &dwarf_decl_ctx) const;

    const   DWARFDebugInfoEntry*    GetParentDeclContextDIE (SymbolFileDWARF* dwarf2Data, 
                                                             DWARFCompileUnit* cu) const;
    const   DWARFDebugInfoEntry*    GetParentDeclContextDIE (SymbolFileDWARF* dwarf2Data, 
                                                             DWARFCompileUnit* cu, 
                                                             const DWARFDebugInfoEntry::Attributes& attributes) const;

    void        
    SetParent (DWARFDebugInfoEntry* parent)     
    {
        if (parent)
        {
            // We know we are kept in a vector of contiguous entries, so we know
            // our parent will be some index behind "this".
            m_parent_idx = this - parent;
        }
        else        
            m_parent_idx = 0;
    }
    void
    SetSibling (DWARFDebugInfoEntry* sibling)
    {
        if (sibling)
        {
            // We know we are kept in a vector of contiguous entries, so we know
            // our sibling will be some index after "this".
            m_sibling_idx = sibling - this;
            sibling->SetParent(GetParent()); 
        }
        else        
            m_sibling_idx = 0;
    }

    void
    SetSiblingIndex (uint32_t idx)
    {
        m_sibling_idx = idx;
    }
    
    void
    SetParentIndex (uint32_t idx)
    {
        m_parent_idx = idx;
    }

    bool
    GetEmptyChildren () const
    {
        return m_empty_children;
    }

    void
    SetEmptyChildren (bool b)
    {
        m_empty_children = b;
    }

    static void
    DumpDIECollection (lldb_private::Stream &strm,
                       DWARFDebugInfoEntry::collection &die_collection);

protected:
    dw_offset_t m_offset;           // Offset within the .debug_info of the start of this entry
    uint32_t    m_parent_idx;       // How many to subtract from "this" to get the parent. If zero this die has no parent
    uint32_t    m_sibling_idx:31,   // How many to add to "this" to get the sibling.
                m_empty_children:1; // If a DIE says it had children, yet it just contained a NULL tag, this will be set.
    uint32_t    m_abbr_idx:DIE_ABBR_IDX_BITSIZE,
                m_has_children:1,   // Set to 1 if this DIE has children
                m_tag:16;           // A copy of the DW_TAG value so we don't have to go through the compile unit abbrev table
                
};

#endif  // SymbolFileDWARF_DWARFDebugInfoEntry_h_
