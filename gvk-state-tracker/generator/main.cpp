
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

#include "gvk-xml.hpp"
#include "basic-cmd-tracker.generator.hpp"
#include "basic-state-tracker.generator.hpp"
#include "enumerate-state-tracked-objects.generator.hpp"
#include "get-state-tracked-object-create-info.generator.hpp"
#include "get-state-tracked-object-info.generator.hpp"
#include "set-state-tracked-object-name.generator.hpp"
#include "state-tracked-handles.generator.hpp"

int main(int, const char*[])
{
    tinyxml2::XMLDocument xmlDocument;
    auto xmlResult = xmlDocument.LoadFile(GVK_XML_FILE_PATH);
    if (xmlResult == tinyxml2::XML_SUCCESS) {
        gvk::xml::Manifest manifest(xmlDocument);
        gvk::cppgen::BasicCmdTrackerGenerator::generate(manifest);
        gvk::cppgen::BasicStateTrackerGenerator::generate(manifest);
        gvk::cppgen::EnumerateStateTrackedObjectsGenerator::generate(manifest);
        gvk::cppgen::GetStateTrackedObjectCreateInfoGenerator::generate(manifest);
        gvk::cppgen::GetStateTrackedObjectInfoGenerator::generate(manifest);
        gvk::cppgen::SetStateTrackedObjectNameGenerator::generate(manifest);
        gvk::cppgen::StateTrackedHandlesGenerator::generate(manifest);
    }
    return 0;
}
