/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
  * @brief   writeXmlAnswset implementation
  * @file    writeXmlAnswset.cpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    28 Apr 2011
  *
  * @edited
  * 20 Mar 2014
  *
  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "common/utils/compiler_defs.h"
#include "mws/types/GenericAnswer.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/xmlparser/XmlResponseFormatter.hpp"

#include "build-gen/config.h"

#define MWSANSWSET_MAIN_NAME "mws:answset"
#define MWSANSWSET_ANSW_NAME "mws:answ"
#define MWSANSWSET_SUBST_NAME "subst"
#define MWSANSWSET_URI_NAME "uri"
#define MWSANSWSET_XPATH_NAME "xpath"
#define MWSANSWSET_SUBSTPAIR_NAME "mws:substpair"

using namespace std;
using namespace mws;

struct LocalContext {
    FILE* file;
    size_t total_bytes_written;
};

/**
  * @brief Callback function used with an xmlOutputBuffer
  *
  */
static inline int fileXmlOutputWriteCallback(void* _ctxt, const char* data,
                                             int data_size) {
    LocalContext* ctxt = (LocalContext*)_ctxt;
    size_t bytes_written = fwrite(data, sizeof(char), data_size, ctxt->file);
    ctxt->total_bytes_written += bytes_written;
    return bytes_written;
}

namespace mws {
namespace parser {

XmlResponseFormatter XmlResponseFormatter::instance;
const XmlResponseFormatter* RESPONSE_FORMATTER_XML =
    &XmlResponseFormatter::instance;

static const char HTTP_ENCODING[] = "application/xml";

const char* XmlResponseFormatter::getContentType() const {
    return HTTP_ENCODING;
}

int XmlResponseFormatter::writeData(const GenericAnswer* ans,
                                    FILE* output) const {
    const MwsAnswset& answerSet = *((const MwsAnswset*)ans);
    xmlOutputBuffer* outPtr;
    xmlTextWriter* writerPtr;
    size_t qvarNr;
    int ret;
    unsigned int i;
    LocalContext ctxt;

    // Initializing values
    outPtr = nullptr;
    writerPtr = nullptr;
    ret = -1;
    qvarNr = answerSet.qvarNames.size();
    ctxt.file = output;
    ctxt.total_bytes_written = 0;

    if ((outPtr = xmlOutputBufferCreateIO(fileXmlOutputWriteCallback, nullptr,
                                          &ctxt, nullptr)) == nullptr) {
        PRINT_WARN("Error while creating the OutputBuffer\n");
    } else if ((writerPtr = xmlNewTextWriter(outPtr)) == nullptr) {
        PRINT_WARN("Error while creating the TextWriter\n");
    } else if ((ret =
                    xmlTextWriterStartDocument(writerPtr,  // xmlTextWriter
                                               nullptr,   // XML version ("1.0")
                                               nullptr,   // Encoding ("UTF-8")
                                               nullptr))  // Standalone ("yes")
               == -1) {
        PRINT_WARN("Error at xmlTextWriterStartDocument\n");
    } else if ((ret = xmlTextWriterWriteComment(
                    writerPtr,
                    BAD_CAST "MwsAnswset generated by " MWS_BUILD)) == -1) {
        PRINT_WARN("Error at xmlTextWriterStartDocument\n");
    } else if ((ret = xmlTextWriterStartElement(
                    writerPtr, BAD_CAST MWSANSWSET_MAIN_NAME)) == -1) {
        PRINT_WARN("Error at xmlTextWriterStartElement\n");
    } else if ((ret = xmlTextWriterWriteAttribute(
                    writerPtr, BAD_CAST "xmlns:mws",
                    BAD_CAST "http://www.mathweb.org/mws/ns")) == -1) {
        PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
    } else if ((ret = xmlTextWriterWriteAttribute(
                    writerPtr, BAD_CAST "size",
                    BAD_CAST std::to_string(answerSet.answers.size())
                        .c_str())) == -1) {
        PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
    } else if ((ret = xmlTextWriterWriteAttribute(
                    writerPtr, BAD_CAST "total",
                    BAD_CAST std::to_string(answerSet.total).c_str())) == -1) {
        PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
    } else {
        for (auto& answer : answerSet.answers) {
            if ((ret = xmlTextWriterStartElement(
                     writerPtr, BAD_CAST MWSANSWSET_ANSW_NAME)) == -1) {
                PRINT_WARN("Error at xmlTextWriterStartElement\n");
                break;
            } else if ((ret = xmlTextWriterWriteAttribute(
                            writerPtr, BAD_CAST MWSANSWSET_URI_NAME,
                            BAD_CAST answer->uri.c_str())) == -1) {
                PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
                break;
            } else if ((ret = xmlTextWriterWriteAttribute(
                            writerPtr, BAD_CAST MWSANSWSET_XPATH_NAME,
                            BAD_CAST answer->xpath.c_str())) == -1) {
                PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
                break;
            } else {
                // Writing the substitutions
                for (i = 0; i < qvarNr; i++) {
                    string qvarXpath = answer->xpath + answerSet.qvarXpaths[i];
                    if ((ret = xmlTextWriterStartElement(
                             writerPtr, BAD_CAST MWSANSWSET_SUBSTPAIR_NAME)) ==
                        -1) {
                        PRINT_WARN("Error at xmlTextWriterStartElement\n");
                        break;
                    } else if ((ret = xmlTextWriterWriteAttribute(
                                    writerPtr, BAD_CAST "qvar",
                                    BAD_CAST answerSet.qvarNames[i].c_str())) ==
                               -1) {
                        PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
                        break;
                    } else if ((ret = xmlTextWriterWriteAttribute(
                                    writerPtr, BAD_CAST "xpath",
                                    BAD_CAST qvarXpath.c_str())) == -1) {
                        PRINT_WARN("Error at xmlTextWriterWriteAttribute\n");
                        break;
                    } else if ((ret = xmlTextWriterEndElement(writerPtr)) ==
                               -1) {
                        PRINT_WARN("Error at xmlTextWriterEndElement\n");
                        break;
                    }
                }
                // <data> ... </data>
                xmlTextWriterWriteElement(writerPtr, BAD_CAST "data",
                                          BAD_CAST answer->data.c_str());
            }
            if (ret == -1) {
                PRINT_WARN("Error while writing xml substpairs\n");
                break;
            } else if ((ret = xmlTextWriterEndElement(writerPtr)) == -1) {
                PRINT_WARN("Error at xmlTextWriterEndElement\n");
                break;
            }
        }
    }
    if (ret == -1) {
        PRINT_WARN("Error while writing xml answers\n");
    } else if ((ret = xmlTextWriterEndElement(writerPtr)) == -1) {
        PRINT_WARN("Error at xmlTextWriterEndElement\n");
    } else if ((ret = xmlTextWriterEndDocument(writerPtr)) == -1) {
        PRINT_WARN("Error at xmlTextWriterEndDocument\n");
    } else if ((ret = xmlTextWriterFlush(writerPtr)) == -1) {
        PRINT_WARN("Error at xmlTextWriterFlush\n");
    } else {
        ret = 0;
    }

    // Cleaning data
    if (writerPtr) {
        // This also cleans the outPtr
        xmlFreeTextWriter(writerPtr);
    } else if (outPtr) {
        xmlOutputBufferClose(outPtr);
    }

    if (ret == 0) {
        return ctxt.total_bytes_written;
    } else {
        return ret;
    }
}

}  // namespace parser
}  // namespace mws
