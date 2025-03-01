/*
 * OptionsTabBase.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "../gui/InterfaceObjectConfigurable.h"
#include "../../lib/filesystem/ResourcePath.h"

/// The options tab which is shown at the map selection phase.
class OptionsTabBase : public InterfaceObjectConfigurable
{
public:
	OptionsTabBase(const JsonPath & configPath);

	void recreate();
};
