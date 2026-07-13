
/*******************************************************************************

MIT License

Copyright (c) Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#pragma once

#ifdef WIN32

#include "gvk-defines.hpp"
#include "gvk-structures.hpp"
#include "gvk-command-structures.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer.h"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-comparison-operators.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-create-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-deserialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-destroy-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-get-stype.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-serialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"

#else

// TODO : Very annoying that Windows and Linux need different include orders for
//  these...that's a very good indicator that these utilities need a rework

#include "gvk-defines.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer.h"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-enumerations-to-string.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-comparison-operators.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-create-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-deserialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-destroy-copy.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-get-stype.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-serialization.hpp"
#include "gvk-pipeline-explorer/generated/pipeline-explorer-structure-to-string.hpp"
#include "gvk-structures.hpp"
#include "gvk-command-structures.hpp"

#endif

namespace gvk {

template <>
inline const GvkPipelineExplorerTimelineQueryInterval& get_default<GvkPipelineExplorerTimelineQueryInterval>()
{
    static const GvkPipelineExplorerTimelineQueryInterval DefaultPipelineExplorerTimelineQueryInterval {
        /* .mode       = */ GVK_PIPELINE_EXPLORER_QUERY_MODE_TOGGLE,
        /* .delimiter  = */ GVK_COMMAND_STRUCTURE_TYPE_QUEUE_SUBMIT,
        /* .count      = */ 60,
        /* .durationMS = */ 1000.0,
        /* .toggle     = */ VK_TRUE,
    };
    return DefaultPipelineExplorerTimelineQueryInterval;
}

template <>
inline const GvkPipelineExplorerQueryRequestInfo& get_default<GvkPipelineExplorerQueryRequestInfo>()
{
    static const GvkPipelineExplorerQueryRequestInfo DefaultPipelineExplorerQueryRequestInfo {
        /* .sType       = */ gvk::get_stype<GvkPipelineExplorerQueryRequestInfo>(),
        /* .pReportPath = */ nullptr,
        /* .interval    = */ gvk::get_default<GvkPipelineExplorerTimelineQueryInterval>(),
    };
    return DefaultPipelineExplorerQueryRequestInfo;
}

} // namespace gvk
