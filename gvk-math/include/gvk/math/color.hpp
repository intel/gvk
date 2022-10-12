
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

#include "gvk/math/defines.hpp"

#ifdef GVK_GLM_ENABLED

namespace gvk {
namespace math {

struct Color
{
    // FROM : https://docs.microsoft.com/en-us/previous-versions/windows/xna/ff433752(v%3dxnagamestudio.41)

    inline static const glm::vec4 White                { 1.0f,       1.0f,       1.0f,       1.0f }; //!< Constant color White { 1.0f, 1.0f, 1.0f, 1.0f }
    inline static const glm::vec4 Black                { 0.0f,       0.0f,       0.0f,       1.0f }; //!< Constant color Black { 0.0f, 0.0f, 0.0f, 1.0f }
    inline static const glm::vec4 Transparent          { 0.0f,       0.0f,       0.0f,       0.0f }; //!< Constant color Transparent { 0.0f, 0.0f, 0.0f, 0.0f }
    inline static const glm::vec4 AliceBlue            { 0.941177f,  0.972549f,  1.0f,       1.0f }; //!< Constant color AliceBlue { 0.941177f, 0.972549f, 1.0f, 1.0f }
    inline static const glm::vec4 AntiqueWhite         { 0.980392f,  0.921569f,  0.843137f,  1.0f }; //!< Constant color AntiqueWhite { 0.980392f, 0.921569f, 0.843137f, 1.0f}
    inline static const glm::vec4 Aqua                 { 0.0f,       1.0f,       1.0f,       1.0f }; //!< Constant color Aqua { 0.0f, 1.0f, 1.0f, 1.0f }
    inline static const glm::vec4 Aquamarine           { 0.498039f,  1.0f,       0.831373f,  1.0f }; //!< Constant color Aquamarine { 0.498039f, 1.0f, 0.831373f, 1.0f }
    inline static const glm::vec4 Azure                { 0.941177f,  1.0f,       1.0f,       1.0f }; //!< Constant color Azure { 0.941177f, 1.0f, 1.0f, 1.0f }
    inline static const glm::vec4 Beige                { 0.960784f,  0.960784f,  0.862745f,  1.0f }; //!< Constant color Beige { 0.960784f, 0.960784f, 0.862745f, 1.0f }
    inline static const glm::vec4 Bisque               { 1.0f,       0.894118f,  0.768628f,  1.0f }; //!< Constant color Bisque { 1.0f, 0.894118f, 0.768628f, 1.0f }
    inline static const glm::vec4 BlanchedAlmond       { 1.0f,       0.921569f,  0.803922f,  1.0f }; //!< Constant color BlanchedAlmond { 1.0f, 0.921569f, 0.803922f, 1.0f }
    inline static const glm::vec4 Blue                 { 0.0f,       0.0f,       1.0f,       1.0f }; //!< Constant color Blue { 0.0f, 0.0f, 1.0f, 1.0f }
    inline static const glm::vec4 BlueViolet           { 0.541176f,  0.168627f,  0.886275f,  1.0f }; //!< Constant color BlueViolet { 0.541176f, 0.168627f, 0.886275f, 1.0f }
    inline static const glm::vec4 Brown                { 0.647059f,  0.164706f,  0.164706f,  1.0f }; //!< Constant color Brown { 0.647059f, 0.164706f, 0.164706f, 1.0f }
    inline static const glm::vec4 BurlyWood            { 0.870588f,  0.721569f,  0.529412f,  1.0f }; //!< Constant color BurlyWood { 0.870588f, 0.721569f, 0.529412f, 1.0f }
    inline static const glm::vec4 CadetBlue            { 0.372549f,  0.619608f,  0.627451f,  1.0f }; //!< Constant color CadetBlue { 0.372549f, 0.619608f, 0.627451f, 1.0f }
    inline static const glm::vec4 Chartreuse           { 0.498039f,  1.0f,       0.0f,       1.0f }; //!< Constant color Chartreuse { 0.498039f, 1.0f, 0.0f, 1.0f }
    inline static const glm::vec4 Chocolate            { 0.823529f,  0.411765f,  0.117647f,  1.0f }; //!< Constant color Chocolate { 0.823529f, 0.411765f, 0.117647f, 1.0f }
    inline static const glm::vec4 Coral                { 1.0f,       0.498039f,  0.313726f,  1.0f }; //!< Constant color Coral { 1.0f, 0.498039f, 0.313726f, 1.0f }
    inline static const glm::vec4 CornflowerBlue       { 0.392157f,  0.584314f,  0.929412f,  1.0f }; //!< Constant color CornflowerBlue { 0.392157f, 0.584314f, 0.929412f, 1.0f}
    inline static const glm::vec4 Cornsilk             { 1.0f,       0.972549f,  0.862745f,  1.0f }; //!< Constant color Cornsilk { 1.0f, 0.972549f, 0.862745f, 1.0f }
    inline static const glm::vec4 Crimson              { 0.862745f,  0.0784314f, 0.235294f,  1.0f }; //!< Constant color Crimson { 0.862745f, 0.0784314f, 0.235294f, 1.0f }
    inline static const glm::vec4 Cyan                 { 0.0f,       1.0f,       1.0f,       1.0f }; //!< Constant color Cyan { 0.0f, 1.0f, 1.0f, 1.0f }
    inline static const glm::vec4 DarkBlue             { 0.0f,       0.0f,       0.545098f,  1.0f }; //!< Constant color DarkBlue { 0.0f, 0.0f, 0.545098f, 1.0f }
    inline static const glm::vec4 DarkCyan             { 0.0f,       0.545098f,  0.545098f,  1.0f }; //!< Constant color DarkCyan { 0.0f, 0.545098f, 0.545098f, 1.0f }
    inline static const glm::vec4 DarkGoldenrod        { 0.721569f,  0.52549f,   0.0431373f, 1.0f }; //!< Constant color DarkGoldenrod { 0.721569f, 0.52549f, 0.0431373f, 1.0f}
    inline static const glm::vec4 DarkGray             { 0.662745f,  0.662745f,  0.662745f,  1.0f }; //!< Constant color DarkGray { 0.662745f, 0.662745f, 0.662745f, 1.0f }
    inline static const glm::vec4 DarkGreen            { 0.0f,       0.392157f,  0.0f,       1.0f }; //!< Constant color DarkGreen { 0.0f, 0.392157f, 0.0f, 1.0f }
    inline static const glm::vec4 DarkKhaki            { 0.741176f,  0.717647f,  0.419608f,  1.0f }; //!< Constant color DarkKhaki { 0.741176f, 0.717647f, 0.419608f, 1.0f }
    inline static const glm::vec4 DarkMagenta          { 0.545098f,  0.0f,       0.545098f,  1.0f }; //!< Constant color DarkMagenta { 0.545098f, 0.0f, 0.545098f, 1.0f }
    inline static const glm::vec4 DarkOliveGreen       { 0.333333f,  0.419608f,  0.184314f,  1.0f }; //!< Constant color DarkOliveGreen { 0.333333f, 0.419608f, 0.184314f, 1.0f}
    inline static const glm::vec4 DarkOrange           { 1.0f,       0.54902f,   0.0f,       1.0f }; //!< Constant color DarkOrange { 1.0f, 0.54902f, 0.0f, 1.0f }
    inline static const glm::vec4 DarkOrchid           { 0.6f,       0.196078f,  0.8f,       1.0f }; //!< Constant color DarkOrchid { 0.6f, 0.196078f, 0.8f, 1.0f }
    inline static const glm::vec4 DarkRed              { 0.545098f,  0.0f,       0.0f,       1.0f }; //!< Constant color DarkRed { 0.545098f, 0.0f, 0.0f, 1.0f }
    inline static const glm::vec4 DarkSalmon           { 0.913726f,  0.588235f,  0.478431f,  1.0f }; //!< Constant color DarkSalmon { 0.913726f, 0.588235f, 0.478431f, 1.0f }
    inline static const glm::vec4 DarkSeaGreen         { 0.560784f,  0.737255f,  0.545098f,  1.0f }; //!< Constant color DarkSeaGreen { 0.560784f, 0.737255f, 0.545098f, 1.0f}
    inline static const glm::vec4 DarkSlateBlue        { 0.282353f,  0.239216f,  0.545098f,  1.0f }; //!< Constant color DarkSlateBlue { 0.282353f, 0.239216f, 0.545098f, 1.0f}
    inline static const glm::vec4 DarkSlateGray        { 0.184314f,  0.309804f,  0.309804f,  1.0f }; //!< Constant color DarkSlateGray { 0.184314f, 0.309804f, 0.309804f, 1.0f}
    inline static const glm::vec4 DarkTurquoise        { 0.0f,       0.807843f,  0.819608f,  1.0f }; //!< Constant color DarkTurquoise { 0.0f, 0.807843f, 0.819608f, 1.0f }
    inline static const glm::vec4 DarkViolet           { 0.580392f,  0.0f,       0.827451f,  1.0f }; //!< Constant color DarkViolet { 0.580392f, 0.0f, 0.827451f, 1.0f }
    inline static const glm::vec4 DeepPink             { 1.0f,       0.0784314f, 0.576471f,  1.0f }; //!< Constant color DeepPink { 1.0f, 0.0784314f, 0.576471f, 1.0f }
    inline static const glm::vec4 DeepSkyBlue          { 0.0f,       0.74902f,   1.0f,       1.0f }; //!< Constant color DeepSkyBlue { 0.0f, 0.74902f, 1.0f, 1.0f }
    inline static const glm::vec4 DimGray              { 0.411765f,  0.411765f,  0.411765f,  1.0f }; //!< Constant color DimGray { 0.411765f, 0.411765f, 0.411765f, 1.0f }
    inline static const glm::vec4 DodgerBlue           { 0.117647f,  0.564706f,  1.0f,       1.0f }; //!< Constant color DodgerBlue { 0.117647f, 0.564706f, 1.0f, 1.0f }
    inline static const glm::vec4 Firebrick            { 0.698039f,  0.133333f,  0.133333f,  1.0f }; //!< Constant color Firebrick { 0.698039f, 0.133333f, 0.133333f, 1.0f }
    inline static const glm::vec4 FloralWhite          { 1.0f,       0.980392f,  0.941177f,  1.0f }; //!< Constant color FloralWhite { 1.0f, 0.980392f, 0.941177f, 1.0f }
    inline static const glm::vec4 ForestGreen          { 0.133333f,  0.545098f,  0.133333f,  1.0f }; //!< Constant color ForestGreen { 0.133333f, 0.545098f, 0.133333f, 1.0f }
    inline static const glm::vec4 Fuchsia              { 1.0f,       0.0f,       1.0f,       1.0f }; //!< Constant color Fuchsia { 1.0f, 0.0f, 1.0f, 1.0f }
    inline static const glm::vec4 Gainsboro            { 0.862745f,  0.862745f,  0.862745f,  1.0f }; //!< Constant color Gainsboro { 0.862745f, 0.862745f, 0.862745f, 1.0f }
    inline static const glm::vec4 GhostWhite           { 0.972549f,  0.972549f,  1.0f,       1.0f }; //!< Constant color GhostWhite { 0.972549f, 0.972549f, 1.0f, 1.0f }
    inline static const glm::vec4 GearsBlue            { 0.2f,       0.2f,       1.0f,       1.0f }; //!< Constant color GearsBlue { 0.2f, 0.2f, 1.0f, 1.0f }
    inline static const glm::vec4 GearsGreen           { 0.0f,       0.8f,       0.2f,       1.0f }; //!< Constant color GearsGreen { 0.0f, 0.8f, 0.2f, 1.0f }
    inline static const glm::vec4 GearsRed             { 0.8f,       0.1f,       0.0f,       1.0f }; //!< Constant color GearsRed { 0.8f, 0.1f, 0.0f, 1.0f }
    inline static const glm::vec4 Gold                 { 1.0f,       0.843137f,  0.0f,       1.0f }; //!< Constant color Gold { 1.0f, 0.843137f, 0.0f, 1.0f }
    inline static const glm::vec4 Goldenrod            { 0.854902f,  0.647059f,  0.12549f,   1.0f }; //!< Constant color Goldenrod { 0.854902f, 0.647059f, 0.12549f, 1.0f }
    inline static const glm::vec4 Gray                 { 0.501961f,  0.501961f,  0.501961f,  1.0f }; //!< Constant color Gray { 0.501961f, 0.501961f, 0.501961f, 1.0f }
    inline static const glm::vec4 Green                { 0.0f,       1.0f,       0.0f,       1.0f }; //!< Constant color Green { 0.0f, 1.0f, 0.0f, 1.0f }
    inline static const glm::vec4 GreenYellow          { 0.678431f,  1.0f,       0.184314f,  1.0f }; //!< Constant color GreenYellow { 0.678431f, 1.0f, 0.184314f, 1.0f }
    inline static const glm::vec4 HalfGray             { 0.5f,       0.5f,       0.5f,       1.0f }; //!< Constant color HalfGray { 0.5f, 0.5f, 0.5f, 1.0f }
    inline static const glm::vec4 Honeydew             { 0.941177f,  1.0f,       0.941177f,  1.0f }; //!< Constant color Honeydew { 0.941177f, 1.0f, 0.941177f, 1.0f }
    inline static const glm::vec4 HotPink              { 1.0f,       0.411765f,  0.705882f,  1.0f }; //!< Constant color HotPink { 1.0f, 0.411765f, 0.705882f, 1.0f }
    inline static const glm::vec4 IndianRed            { 0.803922f,  0.360784f,  0.360784f,  1.0f }; //!< Constant color IndianRed { 0.803922f, 0.360784f, 0.360784f, 1.0f }
    inline static const glm::vec4 Indigo               { 0.294118f,  0.0f,       0.509804f,  1.0f }; //!< Constant color Indigo { 0.294118f, 0.0f, 0.509804f, 1.0f }
    inline static const glm::vec4 Ivory                { 1.0f,       1.0f,       0.941177f,  1.0f }; //!< Constant color Ivory { 1.0f, 1.0f, 0.941177f, 1.0f }
    inline static const glm::vec4 Khaki                { 0.941177f,  0.901961f,  0.54902f,   1.0f }; //!< Constant color Khaki { 0.941177f, 0.901961f, 0.54902f, 1.0f }
    inline static const glm::vec4 Lavender             { 0.901961f,  0.901961f,  0.980392f,  1.0f }; //!< Constant color Lavender { 0.901961f, 0.901961f, 0.980392f, 1.0f }
    inline static const glm::vec4 LavenderBlush        { 1.0f,       0.941177f,  0.960784f,  1.0f }; //!< Constant color LavenderBlush { 1.0f, 0.941177f, 0.960784f, 1.0f }
    inline static const glm::vec4 LawnGreen            { 0.486275f,  0.988235f,  0.0f,       1.0f }; //!< Constant color LawnGreen { 0.486275f, 0.988235f, 0.0f, 1.0f }
    inline static const glm::vec4 LemonChiffon         { 1.0f,       0.980392f,  0.803922f,  1.0f }; //!< Constant color LemonChiffon { 1.0f, 0.980392f, 0.803922f, 1.0f }
    inline static const glm::vec4 LightBlue            { 0.678431f,  0.847059f,  0.901961f,  1.0f }; //!< Constant color LightBlue { 0.678431f, 0.847059f, 0.901961f, 1.0f }
    inline static const glm::vec4 LightCoral           { 0.941177f,  0.501961f,  0.501961f,  1.0f }; //!< Constant color LightCoral { 0.941177f, 0.501961f, 0.501961f, 1.0f }
    inline static const glm::vec4 LightCyan            { 0.878431f,  1.0f,       1.0f,       1.0f }; //!< Constant color LightCyan { 0.878431f, 1.0f, 1.0f, 1.0f }
    inline static const glm::vec4 LightGoldenrodYellow { 0.980392f,  0.980392f,  0.823529f,  1.0f }; //!< Constant color LightGoldenrodYellow { 0.980392f, 0.980392f, 0.823529f,1.0f}
    inline static const glm::vec4 LightGray            { 0.827451f,  0.827451f,  0.827451f,  1.0f }; //!< Constant color LightGray { 0.827451f, 0.827451f, 0.827451f, 1.0f }
    inline static const glm::vec4 LightGreen           { 0.564706f,  0.933333f,  0.564706f,  1.0f }; //!< Constant color LightGreen { 0.564706f, 0.933333f, 0.564706f, 1.0f }
    inline static const glm::vec4 LightPink            { 1.0f,       0.713726f,  0.756863f,  1.0f }; //!< Constant color LightPink { 1.0f, 0.713726f, 0.756863f, 1.0f }
    inline static const glm::vec4 LightSalmon          { 1.0f,       0.627451f,  0.478431f,  1.0f }; //!< Constant color LightSalmon { 1.0f, 0.627451f, 0.478431f, 1.0f }
    inline static const glm::vec4 LightSeaGreen        { 0.12549f,   0.698039f,  0.666667f,  1.0f }; //!< Constant color LightSeaGreen { 0.12549f, 0.698039f, 0.666667f, 1.0f}
    inline static const glm::vec4 LightSkyBlue         { 0.529412f,  0.807843f,  0.980392f,  1.0f }; //!< Constant color LightSkyBlue { 0.529412f, 0.807843f, 0.980392f, 1.0f}
    inline static const glm::vec4 LightSlateGray       { 0.466667f,  0.533333f,  0.6f,       1.0f }; //!< Constant color LightSlateGray { 0.466667f, 0.533333f, 0.6f, 1.0f }
    inline static const glm::vec4 LightSteelBlue       { 0.690196f,  0.768628f,  0.870588f,  1.0f }; //!< Constant color LightSteelBlue { 0.690196f, 0.768628f, 0.870588f, 1.0f}
    inline static const glm::vec4 LightYellow          { 1.0f,       1.0f,       0.878431f,  1.0f }; //!< Constant color LightYellow { 1.0f, 1.0f, 0.878431f, 1.0f }
    inline static const glm::vec4 Lime                 { 0.0f,       1.0f,       0.0f,       1.0f }; //!< Constant color Lime { 0.0f, 1.0f, 0.0f, 1.0f }
    inline static const glm::vec4 LimeGreen            { 0.196078f,  0.803922f,  0.196078f,  1.0f }; //!< Constant color LimeGreen { 0.196078f, 0.803922f, 0.196078f, 1.0f }
    inline static const glm::vec4 Linen                { 0.980392f,  0.941177f,  0.901961f,  1.0f }; //!< Constant color Linen { 0.980392f, 0.941177f, 0.901961f, 1.0f }
    inline static const glm::vec4 Magenta              { 1.0f,       0.0f,       1.0f,       1.0f }; //!< Constant color Magenta { 1.0f, 0.0f, 1.0f, 1.0f }
    inline static const glm::vec4 Maroon               { 0.501961f,  0.0f,       0.0f,       1.0f }; //!< Constant color Maroon { 0.501961f, 0.0f, 0.0f, 1.0f }
    inline static const glm::vec4 MediumAquamarine     { 0.4f,       0.803922f,  0.666667f,  1.0f }; //!< Constant color MediumAquamarine { 0.4f, 0.803922f, 0.666667f, 1.0f }
    inline static const glm::vec4 MediumBlue           { 0.0f,       0.0f,       0.803922f,  1.0f }; //!< Constant color MediumBlue { 0.0f, 0.0f, 0.803922f, 1.0f }
    inline static const glm::vec4 MediumOrchid         { 0.729412f,  0.333333f,  0.827451f,  1.0f }; //!< Constant color MediumOrchid { 0.729412f, 0.333333f, 0.827451f, 1.0f}
    inline static const glm::vec4 MediumPurple         { 0.576471f,  0.439216f,  0.858824f,  1.0f }; //!< Constant color MediumPurple { 0.576471f, 0.439216f, 0.858824f, 1.0f}
    inline static const glm::vec4 MediumSeaGreen       { 0.235294f,  0.701961f,  0.443137f,  1.0f }; //!< Constant color MediumSeaGreen { 0.235294f, 0.701961f, 0.443137f, 1.0f}
    inline static const glm::vec4 MediumSlateBlue      { 0.482353f,  0.407843f,  0.933333f,  1.0f }; //!< Constant color MediumSlateBlue { 0.482353f, 0.407843f, 0.933333f, 1.0f}
    inline static const glm::vec4 MediumSpringGreen    { 0.0f,       0.980392f,  0.603922f,  1.0f }; //!< Constant color MediumSpringGreen { 0.0f, 0.980392f, 0.603922f, 1.0f}
    inline static const glm::vec4 MediumTurquoise      { 0.282353f,  0.819608f,  0.8f,       1.0f }; //!< Constant color MediumTurquoise { 0.282353f, 0.819608f, 0.8f, 1.0f }
    inline static const glm::vec4 MediumVioletRed      { 0.780392f,  0.0823529f, 0.521569f,  1.0f }; //!< Constant color MediumVioletRed { 0.780392f, 0.0823529f, 0.521569f, 1.0f}
    inline static const glm::vec4 MidnightBlue         { 0.0980392f, 0.0980392f, 0.439216f,  1.0f }; //!< Constant color MidnightBlue { 0.0980392f, 0.0980392f, 0.439216f, 1.0f}
    inline static const glm::vec4 MintCream            { 0.960784f,  1.0f,       0.980392f,  1.0f }; //!< Constant color MintCream { 0.960784f, 1.0f, 0.980392f, 1.0f }
    inline static const glm::vec4 MistyRose            { 1.0f,       0.894118f,  0.882353f,  1.0f }; //!< Constant color MistyRose { 1.0f, 0.894118f, 0.882353f, 1.0f }
    inline static const glm::vec4 Moccasin             { 1.0f,       0.894118f,  0.709804f,  1.0f }; //!< Constant color Moccasin { 1.0f, 0.894118f, 0.709804f, 1.0f }
    inline static const glm::vec4 NavajoWhite          { 1.0f,       0.870588f,  0.678431f,  1.0f }; //!< Constant color NavajoWhite { 1.0f, 0.870588f, 0.678431f, 1.0f }
    inline static const glm::vec4 Navy                 { 0.0f,       0.0f,       0.501961f,  1.0f }; //!< Constant color Navy { 0.0f, 0.0f, 0.501961f, 1.0f }
    inline static const glm::vec4 OldLace              { 0.992157f,  0.960784f,  0.901961f,  1.0f }; //!< Constant color OldLace { 0.992157f, 0.960784f, 0.901961f, 1.0f }
    inline static const glm::vec4 Olive                { 0.501961f,  0.501961f,  0.0f,       1.0f }; //!< Constant color Olive { 0.501961f, 0.501961f, 0.0f, 1.0f }
    inline static const glm::vec4 OliveDrab            { 0.419608f,  0.556863f,  0.137255f,  1.0f }; //!< Constant color OliveDrab { 0.419608f, 0.556863f, 0.137255f, 1.0f }
    inline static const glm::vec4 Orange               { 1.0f,       0.647059f,  0.0f,       1.0f }; //!< Constant color Orange { 1.0f, 0.647059f, 0.0f, 1.0f }
    inline static const glm::vec4 OrangeRed            { 1.0f,       0.270588f,  0.0f,       1.0f }; //!< Constant color OrangeRed { 1.0f, 0.270588f, 0.0f, 1.0f }
    inline static const glm::vec4 Orchid               { 0.854902f,  0.439216f,  0.839216f,  1.0f }; //!< Constant color Orchid { 0.854902f, 0.439216f, 0.839216f, 1.0f }
    inline static const glm::vec4 PaleGoldenrod        { 0.933333f,  0.909804f,  0.666667f,  1.0f }; //!< Constant color PaleGoldenrod { 0.933333f, 0.909804f, 0.666667f, 1.0f}
    inline static const glm::vec4 PaleGreen            { 0.596078f,  0.984314f,  0.596078f,  1.0f }; //!< Constant color PaleGreen { 0.596078f, 0.984314f, 0.596078f, 1.0f }
    inline static const glm::vec4 PaleTurquoise        { 0.686275f,  0.933333f,  0.933333f,  1.0f }; //!< Constant color PaleTurquoise { 0.686275f, 0.933333f, 0.933333f, 1.0f}
    inline static const glm::vec4 PaleVioletRed        { 0.858824f,  0.439216f,  0.576471f,  1.0f }; //!< Constant color PaleVioletRed { 0.858824f, 0.439216f, 0.576471f, 1.0f}
    inline static const glm::vec4 PapayaWhip           { 1.0f,       0.937255f,  0.835294f,  1.0f }; //!< Constant color PapayaWhip { 1.0f, 0.937255f, 0.835294f, 1.0f }
    inline static const glm::vec4 PeachPuff            { 1.0f,       0.854902f,  0.72549f,   1.0f }; //!< Constant color PeachPuff { 1.0f, 0.854902f, 0.72549f, 1.0f }
    inline static const glm::vec4 Peru                 { 0.803922f,  0.521569f,  0.247059f,  1.0f }; //!< Constant color Peru { 0.803922f, 0.521569f, 0.247059f, 1.0f }
    inline static const glm::vec4 Pink                 { 1.0f,       0.752941f,  0.796079f,  1.0f }; //!< Constant color Pink { 1.0f, 0.752941f, 0.796079f, 1.0f }
    inline static const glm::vec4 Plum                 { 0.866667f,  0.627451f,  0.866667f,  1.0f }; //!< Constant color Plum { 0.866667f, 0.627451f, 0.866667f, 1.0f }
    inline static const glm::vec4 PowderBlue           { 0.690196f,  0.878431f,  0.901961f,  1.0f }; //!< Constant color PowderBlue { 0.690196f, 0.878431f, 0.901961f, 1.0f }
    inline static const glm::vec4 Purple               { 0.501961f,  0.0f,       0.501961f,  1.0f }; //!< Constant color Purple { 0.501961f, 0.0f, 0.501961f, 1.0f }
    inline static const glm::vec4 QuarterGray          { 0.25f,      0.25f,      0.25f,      1.0f }; //!< Constant color QuarterGray { 0.25f, 0.25f, 0.25f, 1.0f }
    inline static const glm::vec4 Red                  { 1.0f,       0.0f,       0.0f,       1.0f }; //!< Constant color Red { 1.0f, 0.0f, 0.0f, 1.0f }
    inline static const glm::vec4 RosyBrown            { 0.737255f,  0.560784f,  0.560784f,  1.0f }; //!< Constant color RosyBrown { 0.737255f, 0.560784f, 0.560784f, 1.0f }
    inline static const glm::vec4 RoyalBlue            { 0.254902f,  0.411765f,  0.882353f,  1.0f }; //!< Constant color RoyalBlue { 0.254902f, 0.411765f, 0.882353f, 1.0f }
    inline static const glm::vec4 SaddleBrown          { 0.545098f,  0.270588f,  0.0745098f, 1.0f }; //!< Constant color SaddleBrown { 0.545098f, 0.270588f, 0.0745098f, 1.0f}
    inline static const glm::vec4 Salmon               { 0.980392f,  0.501961f,  0.447059f,  1.0f }; //!< Constant color Salmon { 0.980392f, 0.501961f, 0.447059f, 1.0f }
    inline static const glm::vec4 SandyBrown           { 0.956863f,  0.643137f,  0.376471f,  1.0f }; //!< Constant color SandyBrown { 0.956863f, 0.643137f, 0.376471f, 1.0f }
    inline static const glm::vec4 SeaGreen             { 0.180392f,  0.545098f,  0.341176f,  1.0f }; //!< Constant color SeaGreen { 0.180392f, 0.545098f, 0.341176f, 1.0f }
    inline static const glm::vec4 SeaShell             { 1.0f,       0.960784f,  0.933333f,  1.0f }; //!< Constant color SeaShell { 1.0f, 0.960784f, 0.933333f, 1.0f }
    inline static const glm::vec4 Sienna               { 0.627451f,  0.321569f,  0.176471f,  1.0f }; //!< Constant color Sienna { 0.627451f, 0.321569f, 0.176471f, 1.0f }
    inline static const glm::vec4 Silver               { 0.752941f,  0.752941f,  0.752941f,  1.0f }; //!< Constant color Silver { 0.752941f, 0.752941f, 0.752941f, 1.0f }
    inline static const glm::vec4 SkyBlue              { 0.529412f,  0.807843f,  0.921569f,  1.0f }; //!< Constant color SkyBlue { 0.529412f, 0.807843f, 0.921569f, 1.0f }
    inline static const glm::vec4 SlateBlue            { 0.415686f,  0.352941f,  0.803922f,  1.0f }; //!< Constant color SlateBlue { 0.415686f, 0.352941f, 0.803922f, 1.0f }
    inline static const glm::vec4 SlateGray            { 0.439216f,  0.501961f,  0.564706f,  1.0f }; //!< Constant color SlateGray { 0.439216f, 0.501961f, 0.564706f, 1.0f }
    inline static const glm::vec4 Snow                 { 1.0f,       0.980392f,  0.980392f,  1.0f }; //!< Constant color Snow { 1.0f, 0.980392f, 0.980392f, 1.0f }
    inline static const glm::vec4 SpringGreen          { 0.0f,       1.0f,       0.498039f,  1.0f }; //!< Constant color SpringGreen { 0.0f, 1.0f, 0.498039f, 1.0f }
    inline static const glm::vec4 SteelBlue            { 0.27451f,   0.509804f,  0.705882f,  1.0f }; //!< Constant color SteelBlue { 0.27451f, 0.509804f, 0.705882f, 1.0f }
    inline static const glm::vec4 Tan                  { 0.823529f,  0.705882f,  0.54902f,   1.0f }; //!< Constant color Tan { 0.823529f, 0.705882f, 0.54902f, 1.0f }
    inline static const glm::vec4 Teal                 { 0.0f,       0.501961f,  0.501961f,  1.0f }; //!< Constant color Teal { 0.0f, 0.501961f, 0.501961f, 1.0f }
    inline static const glm::vec4 ThirdGray            { 0.3f,       0.3f,       0.3f,       1.0f }; //!< Constant color ThirdGray { 0.3f, 0.3f, 0.3f, 1.0f }
    inline static const glm::vec4 Thistle              { 0.847059f,  0.74902f,   0.847059f,  1.0f }; //!< Constant color Thistle { 0.847059f, 0.74902f, 0.847059f, 1.0f }
    inline static const glm::vec4 Tomato               { 1.0f,       0.388235f,  0.278431f,  1.0f }; //!< Constant color Tomato { 1.0f, 0.388235f, 0.278431f, 1.0f }
    inline static const glm::vec4 Turquoise            { 0.25098f,   0.878431f,  0.815686f,  1.0f }; //!< Constant color Turquoise { 0.25098f, 0.878431f, 0.815686f, 1.0f }
    inline static const glm::vec4 Violet               { 0.933333f,  0.509804f,  0.933333f,  1.0f }; //!< Constant color Violet { 0.933333f, 0.509804f, 0.933333f, 1.0f }
    inline static const glm::vec4 Wheat                { 0.960784f,  0.870588f,  0.701961f,  1.0f }; //!< Constant color Wheat { 0.960784f, 0.870588f, 0.701961f, 1.0f }
    inline static const glm::vec4 WhiteSmoke           { 0.960784f,  0.960784f,  0.960784f,  1.0f }; //!< Constant color WhiteSmoke { 0.960784f, 0.960784f, 0.960784f, 1.0f }
    inline static const glm::vec4 Yellow               { 1.0f,       1.0f,       0.0f,       1.0f }; //!< Constant color Yellow { 1.0f, 1.0f, 0.0f, 1.0f }
    inline static const glm::vec4 YellowGreen          { 0.603922f,  0.803922f,  0.196078f,  1.0f }; //!< Constant color YellowGreen { 0.603922f, 0.803922f, 0.196078f, 1.0f }

    struct BlueSteel
    {
        inline static const glm::vec4 base { 0.329412f, 0.560784f, 0.678431f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.254902f, 0.447059f, 0.541176f, 1.0f },
            glm::vec4 { 0.094118f, 0.207843f, 0.266667f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.525490f, 0.701961f, 0.792157f, 1.0f },
            glm::vec4 { 0.725490f, 0.839216f, 0.898039f, 1.0f },
        };
    };

    struct Carbon
    {
        inline static const glm::vec4 base { 0.501961f, 0.501961f, 0.501961f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.321569f, 0.321569f, 0.321569f, 1.0f },
            glm::vec4 { 0.149020f, 0.149020f, 0.149020f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.682353f, 0.682353f, 0.682353f, 1.0f },
            glm::vec4 { 0.913725f, 0.913725f, 0.913725f, 1.0f },
        };
    };

    struct ClassicBlue
    {
        inline static const glm::vec4 base { 0.0f, 0.407843f, 0.709804f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.0f, 0.290196f, 0.525490f, 1.0f },
            glm::vec4 { 0.0f, 0.156863f, 0.352941f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.0f, 0.639216f, 0.964706f, 1.0f },
            glm::vec4 { 0.462745f, 0.807843f, 1.0f, 1.0f },
        };
    };

    struct Cobalt
    {
        inline static const glm::vec4 base { 0.117647f, 0.180392f, 0.721569f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.0f, 0.058824f, 0.541176f, 1.0f },
            glm::vec4 { 0.0f, 0.031373f, 0.392157f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.356863f, 0.411765f, 1.0f, 1.0f },
            glm::vec4 { 0.596078f, 0.631373f, 1.0f, 1.0f },
        };
    };

    struct Coral
    {
        inline static const glm::vec4 base { 1.0f, 0.337255f, 0.384314f, 1.0f };
        inline static const std::array<glm::vec4, 1> shades {
            glm::vec4 { 0.784314f, 0.074510f, 0.149020f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 1.0f, 0.517647f, 0.541176f, 1.0f },
            glm::vec4 { 1.0f, 0.713726f, 0.725490f, 1.0f },
        };
    };

    struct Daisy
    {
        inline static const glm::vec4 base { 0.996078f, 0.788235f, 0.105882f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.929412f, 0.698039f, 0.0f, 1.0f },
            glm::vec4 { 0.788235f, 0.560784f, 0.0f, 1.0f },
        };
        inline static const std::array<glm::vec4, 1> tints {
            glm::vec4 { 1.0f, 0.882353f, 0.478431f, 1.0f },
        };
    };

    struct EnergyBlue
    {
        inline static const glm::vec4 base { 0.0f, 0.780392f, 0.992157f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.0f, 0.584314f, 0.792157f, 1.0f },
            glm::vec4 { 0.0f, 0.356863f, 0.521569f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.482353f, 0.870588f, 1.0f, 1.0f },
            glm::vec4 { 0.705882f, 0.941176f, 1.0f, 1.0f },
        };
    };

    struct Geode
    {
        inline static const glm::vec4 base { 0.560784f, 0.364706f, 0.635294f, 1.0f };
        inline static const std::array<glm::vec4, 1> shades {
            glm::vec4 { 0.396078f, 0.192157f, 0.443137f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.800000f, 0.580392f, 0.854902f, 1.0f },
            glm::vec4 { 0.933333f, 0.764706f, 0.968627f, 1.0f },
        };
    };

    struct Moss
    {
        inline static const glm::vec4 base { 0.545098f, 0.682353f, 0.274510f, 1.0f };
        inline static const std::array<glm::vec4, 2> shades {
            glm::vec4 { 0.439216f, 0.521569f, 0.254902f, 1.0f },
            glm::vec4 { 0.317647f, 0.352941f, 0.239216f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 0.694118f, 0.823529f, 0.447059f, 1.0f },
            glm::vec4 { 0.843137f, 0.952941f, 0.635294f, 1.0f },
        };
    };

    struct Rust
    {
        inline static const glm::vec4 base { 0.913725f, 0.380392f, 0.082353f, 1.0f };
        inline static const std::array<glm::vec4, 1> shades {
            glm::vec4 { 0.698039f, 0.270588f, 0.003922f, 1.0f },
        };
        inline static const std::array<glm::vec4, 2> tints {
            glm::vec4 { 1.0f, 0.560784f, 0.317647f, 1.0f },
            glm::vec4 { 1.0f, 0.772549f, 0.600000f, 1.0f },
        };
    };
};

} // namespace math
} // namespace gvk

#endif // GVK_GLM_ENABLED
