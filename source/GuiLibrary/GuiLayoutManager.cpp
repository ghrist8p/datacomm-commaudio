#include "GuiLayoutManager.h"
#include "GuiResizable.h"

void GuiLayoutManager::addExternalComponent(GuiResizable *guiComponent)
{
	externalComponents.insert(guiComponent);
}

void GuiLayoutManager::resizeExternalComponents()
{
	set<GuiResizable*>::iterator itr;

	for (itr = externalComponents.begin(); itr != externalComponents.end(); itr++)
	{
		(*itr)->resize();
	}
}