/*
  XMEM LCD Library for the Arduino

  Copyright 2012,2013 Andrew Brown

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This notice may not be removed or altered from any source distribution.
*/

/**
 * @file ILI9327Colour.h
 * @brief Generic template definition.
 * @ingroup ILI9327
 */

#pragma once


namespace lcd {


	/**
	 * @brief Forward declaration of the ILI9327Colour template
	 *
	 * Empty generic class definition for ILI9327 colour-dependent operations
	 * Everything is provided in the specialisations.
	 *
	 * @ingroup ILI9327
	 */

	template<ColourDepth TColour,class TAccessMode>
	class ILI9327Colour;
}


/*
 * Now include the specialisations
 */

#include "ILI9327Colour16Specialisation.h"
