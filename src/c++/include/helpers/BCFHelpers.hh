// -*- mode: c++; indent-tabs-mode: nil; -*-
//
// 
// Copyright (c) 2010-2015 Illumina, Inc.
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



/**
 *  \brief C++ Helper functions for HTSlib
 *
 *
 * \file BCFHelpers.hh
 * \author Peter Krusche
 * \email pkrusche@illumina.com
 *
 */

#pragma once

#include "../Variant.hh"

extern "C" {

// GCC warns us about some things in htslib here. We don't care.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"

#include <htslib/vcf.h>
#include <htslib/synced_bcf_reader.h>
#include <htslib/vcfutils.h>

#pragma GCC diagnostic pop
}

namespace bcfhelpers {

    /** when things go wrong, this gets thrown */
    struct importexception : public std::runtime_error {
        importexception(std::string const what) : std::runtime_error(what) {}
    };

    /**
     * @brief Set hg19 contig names in header.
     * @param header BCF header
     */
    void bcfHeaderHG19(bcf_hdr_t * header);

    /* get list of sample names from BCF header */
    std::list<std::string> getSampleNames(const bcf_hdr_t * hdr);

    /** shortcut to get chromosome name */
    static inline std::string getChrom(const bcf_hdr_t * hdr, const bcf1_t * rec)
    {
        return hdr->id[BCF_DT_CTG][rec->rid].key;
    }

    /** extract pos / length */
    void getLocation(bcf_hdr_t * hdr, bcf1_t * rec, int64_t & start, int64_t & end);

    /**
     * @brief Retrieve an info field as an integer
     * @details [long description]
     *
     * @param result the default to return if the field is not present
     */
    std::string getInfoString(bcf_hdr_t * header, bcf1_t * line, const char * field, const char * def_result = ".");

    /**
     * @brief Retrieve an info field as an integer
     * @details [long description]
     *
     * @param result the default to return if the field is not present
     */
    int getInfoInt(bcf_hdr_t * header, bcf1_t * line, const char * field, int result = -1);

    /**
     * @brief Retrieve an info field as a double
     *
     * @return the value or NaN
     */
    double getInfoDouble(bcf_hdr_t * header, bcf1_t * line, const char * field);

    /**
     * @brief Retrieve an info flag
     *
     * @return true of the flag is set
     */
    bool getInfoFlag(bcf_hdr_t * header, bcf1_t * line, const char * field);

    /**
     * @brief Read the GT field
     */
    void getGT(bcf_hdr_t * header, bcf1_t * line, int isample, int * gt, int & ngt, bool & phased);

    /** read GQ(X) -- will use in this order: GQ, GQX, -1 */
    void getGQ(const bcf_hdr_t * header, bcf1_t * line, int isample, float & gq);

    /** read AD */
    void getAD(const bcf_hdr_t * header, bcf1_t * line, int isample, int *ad, int max_ad);

    /** read DP(I) -- will use in this order: DP, DPI, -1 */
    void getDP(const bcf_hdr_t * header, bcf1_t * line, int isample, int & dp);

} // namespace bcfhelpers
