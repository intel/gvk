
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

#include "cerealize-structures.generator.hpp"
#include "comparison-operators.generator.hpp"
#include "create-structure-copy.generator.hpp"
#include "decerealize-structures.generator.hpp"
#include "deserialize-structures.generator.hpp"
#include "destroy-structure-copy.generator.hpp"
#include "get-stype.generator.hpp"
#include "make-tuple.generator.hpp"
#include "serialize-structures.generator.hpp"
#include "structure-to-string.generator.hpp"

#include <vector>

namespace gvk {
namespace cppgen {

class StructureUtilitiesGenerator final
{
public:
    static void generate(
        const std::string& structureCollectionName,
        const std::string& structureCollectionInclude,
        const xml::Manifest& manifest,
        const std::vector<xml::Structure>& structures
    )
    {
        CerealizeStructureGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
        ComparisonOperatorsGenerator::generate(structureCollectionName, structureCollectionInclude, structures);
        CreateStructureCopyGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
        DecerealizeStructureGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
        DeserializeStructureGenerator::generate(structureCollectionName, structureCollectionInclude, structures);
        DestroyStructureCopyGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
        GetSTypeGenerator::generate(structureCollectionName, structureCollectionInclude, structures);
        MakeTupleGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
        SerializeStructureGenerator::generate(structureCollectionName, structureCollectionInclude, structures);
        StructureToStringGenerator::generate(structureCollectionName, structureCollectionInclude, manifest, structures);
    }
};

} // namespace cppgen
} // namespace gvk
