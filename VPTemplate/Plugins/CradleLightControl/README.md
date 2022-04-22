# VirtualProductionTools

This repo is meant to serve as a single place for all tools developed for the purposes of virtual production.

## Light controls

The light controls are part of the CradleLightControl plugin that is part of this project. These tools were developed with the goal of streamlining the controls of both virtual lights in the Unreal Engine 4 scene, as well as any physical lights controlled via DMX.

###### Note: As of November 2021, these tools are largely finished, but still in development. It is not unlikely that they may crash the engine during runtime. If that happens, please provide us with the crash log so we can investigate the issue and correct it.

### General overview
The two tools were developed to strongly resemble each other both visually and functionally. As such, the bulk of the features are shared between them, with the only differences being enforced due to the nature of the lights they work with. This section will focus only on these features, and any differences will be covered into their own sections.

 - **General light features:** at the core of the tools is the concept of a Light Item. These objects represent a virtual or physical light, and affect the color, intensity, temperature, and any light specific properties, such as orientation for directional and spot lights, or the cone angle for spot lights. The naming of the lights in the tool is completely cosmetic, and is not reflected the editor in any other way. Additionally, light can be given clarifying notes.
 - **Group hierarchy:** the tools come with a group hierarchy which allows the user to organize the lights based on their needs. It is possible for groups to contain other groups, so there is no limit to how many layers of groups are used. Selected groups are meant to streamline the editting of the properties for multiple lights. Lights and groups are assigned to groups by draging and dropping them onto a group. New groups can be added either manually via the button, or automatically by dragging a light item onto another item. 

![Tool Overview](https://user-images.githubusercontent.com/57301959/141966742-ecfa2046-1bd1-4e19-b1b4-69fc0420d511.png)

 - **Master light concept:** if a group encompassing multiple lights is selected, or simply multiple lights through multiple selection, one of these lights can be declared as the master light. This light's properties will be displayed in the tool, and any changes to its properties will be reflected to all other selected lights. In the case of the orientation properties for directional and spot lights, the relative rotation between the lights is preserved.
 - **Search bar:** the search bar that comes with the group hierarchy filters out any items and groups that match the search criteria. The search criteria is not case sensitive. If a group contains items which match the search criteria, that group will remain visible.
 - **Undo and redo:** actions taken via the tools are recorded in Unreal Engine's transaction system, on the same layer as edit done within any of its other editors, and as such these changes can be undone or redone in the same fashion. The actions accounted for encompass most of the features of the tool, but it is not unlikely that some actions were missed or omitted, so we would appreciate it if we could receive word of any such actions.
 - **Saving and loading of presets:** the state of both tools can be saved to a JSON table file, capturing the hierarchy and state of each light item at the time of the save. These properties can be recovered at any time by loading that same JSON file. Upon loading the file, any DMX lights or virtual lights will have these changes applied to them automatically. These actions can be taken using the buttons under the search bar. The tools also support periodic autosaving of their state, either to a default file or to the last used file. Reopening the tools will also automatically load the state from the last used file, or the autosave. 
 - **Gel palette:** the tools come with a Gel palette that can be used to quickly select a hue, saturation and intensity for the selected lights. The gels available are based on the ones available from Lee Gels. The palette opens in a separate window, and remains until closed via the close button or by double clicking an item in it, which will apply the item and close it automatically. Single clicks on the items can be used to preview gels.

![Gel Palette window](https://user-images.githubusercontent.com/57301959/141967091-7caf3331-0c4e-414e-940e-d4cd6d12dd50.png)

 
Both the DMX controls and virtual light controls have several specifics in their UI. The most notable of those are special buttons under the group hierarchy depending on the usage of the tool, as well as specific properties under the light detail sliders.

### Virtual Light specifics
 The UI for the virtual lights does not add much functionally on top of the general features. There's two main additions:
  - **Regenerate tool data button:** This button is meant as a means of resetting the tool to a state based on the lights in the currently open level. This will remove all groups, notes and used assigned names, and replace all light items with the ones found in the level. Note that this button does not need to be used when you add a new light to the scene, as the tool will detect that and automatically add it to its hierarchy.
  - **Transform viewer:** This UI can be found underneath the light properties when a light is selected. Its main purpose is to inform of the transform of the selected light by reporting its position, rotation and scale, as well as its parent actor if such exists. It also includes a button to quickly select the light in the level editor, or its parent actor.
 
 ![Transform viewer UI](https://user-images.githubusercontent.com/57301959/141966983-c4889200-4fbe-47d6-92ab-5b801464096b.png)


The bulk of the differences for the virtual lights come in the form of automatic processes for verification or ease of use. These differences are as follows:
 - **Periodic item verification:** as the engine does not provide an easy way of detecting when an observed actor is destroyed or modified, the tool instead periodically will go through all lights it contains to verify they haven't been deleted, and to reflect any changes made in-editor within the tool. This period is set to once every couple of seconds. While this is a short period, it does not guarantee that the light items in the tool are not valid at all times. Light items which were deleted in the editor but still present in the tool can be identified by their on/off button being tinted yellow rather than being fully white or greyed out.
 - **Automatic preset loading based on map:** because the lights in the tool are dependent on the lights in the opened map, the virtual light tools will save a different preset file to use for each map it is used in. This will only work if the tool state was excplicitly saved in a map, otherwise the tool will instead regenerate its data from scratch upon the map being opened.

### DMX Light UI specifics
 Similar to the virtual lights, the DMX lights only add a button and a custom section in the UI. These are as follows:
  - **Add DMX Light button:** seeing as the engine does not receive any information about how many DMX fixtues are connected to the machine, it is up to the user to define that. This is done by adding light items through this button. There is no limit to how many lights can be present in the tool, and a single light item can control multiple DMX light if they interpret their DMX channels the same way.
  - **DMX property editor:** this section can be found under the light color properties when a DMX light is selected. These properties are used to define how this light item interacts with the DMX protocol. For more details on what all these properties do, see below.

The DMX light tool does not include any of the automatic processes that the virtual lights do, as it is not dependent on the level it is used in.

### DMX System Usage
Because DMX baseline is not much more than a large number of 8-bit values, we have developed an additional layer to make its usage more intuitive for lights. This system is meant to be used via the DMX Light tool, as well as a simple editor for the assets that it uses.

#### DMX properties
As mentioned above, the DMX Light tool has a section dedicated to DMX properties. These properties define how the light item should send signals through DMX when any light properties change. The properties are as follows:
 - **Output port:** this determines through which output port the signal should be sent. Note that if the port was connected after the engine was started, you will need to restart Unreal Engine for it to connect to the port.
 - **Use DMX checkbox:** controls whether DMX signals are sent out by this light item when changes are made to its properties.
 - **Starting channel:** this determines what the first channel for the signal will be. This will very likely be the value of the channel that your DMX light is set to listen to.
 - **DMX Config:** this defines which DMX Config asset to use for this item. DMX Config is a custom asset type added to the engine for this system, see below for more details on its usage.

![DMX properties](https://user-images.githubusercontent.com/57301959/141967846-b95fc274-43a9-4443-8851-a68175db37bc.png)


If the output port and DMX config are selected and valid, the light item is ready to send out signals, and assuming it is set to use DMX, it should immediately send out a signal. To verify if that was successful you can use the DMX Output buffer tool that comes with Unreal Engine's DMX plugin.

#### DMX Config Assets
DMX Configs are custom assets added to the engine for this system. These are used to define how the different light properties are converted into DMX values and what channels to send these values to. It is expected that for each different light model you use, you will need a new DMX Config asset.

To create a DMX Config asset, right-click in the content browser to open the new asset menu. The DMX Config can be found under the DMX category.

![DMX Config asset creation](https://user-images.githubusercontent.com/57301959/141968299-92e82519-b14a-49cf-aaa7-ca56e1494d64.png)

Each DMX Config defines several properties that can be mapped onto DMX Channels:
 - On/off state - Toggle channel
 - Vertical and horizontal rotation - Linear channels
 - Red, green and blue color channels - Linear channels
 - Intensity - Linear channel

![DMX Config asset editor view](https://user-images.githubusercontent.com/57301959/141968630-8502cb0e-a313-4258-8f05-2a9e200ecbe3.png)


It is also possible to define constant values for specific channels. This is done via the Constant channels array. These channels will be applied first, so they will be overwritten by any properties that share the same channel.

All non-constant channels include two common properties - the channel index and whether it's enabled. The channel index determines the offset from the starting channel of the DMX Light Item. Consult the DMX manual for the light you are using to determine the correct value for each channel index. 

Note that the index is **not** zero-based (the first index is 1, instead of 0). This means that if the first channel in your light's manual is indexed as 0, you will need to add 1 to the channel indices you use to ensure it is correct.

The non-constant channels are divided in two types: Linear and Toggle channels.
Linear channels are meant for properties which interpolate between values, such as the intensity or the color channels. This is done by defining two value ranges - one for DMX and one for the property itself. For example, if the channel is used for the horizontal rotation of a wash light, it's DMX value range will be from 0 to 255, while the property value range will be from 0 to 360. This way, if the property is 0, the channel will be set to 0. On the other hand, if property is 360, the channel will be set to 255. Any intermediate property value will interpolate the DMX value in the same manner.

Toggle channels are meant to be used for properties which only have 2 possible states, e.g. whether the light is on or off. Each toggle channel defines an On and Off DMX value.
Because such properties might use the same DMX channel as other ones, it is possible to set a toggle channel only apply when it's on or off, rather than always. For example, if the intensity of the light is defined in the same channel as whether or not the light is on or off, you might want the on/off property to only apply when it is off so that it doesn't overwrite the intensity when the light is turned on.
