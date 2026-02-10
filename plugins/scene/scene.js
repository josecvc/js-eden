/**
 * JS-Eden Scene Plugin
 * Allows a Scene to be displayed and used within JS-Eden for drawing in 3D.
 * @class Scene Plugin
 */
EdenUI.plugins.Scene = function (edenUI, success) {
    edenUI.views["Scene"] = {dialog: this.createDialog, embedded: this.createEmbedded, title: "Scene", category: edenUI.viewCategories.visualization, holdsContent: true};
}

EdenUI.plugins.Canvas2D.title = "Scene";
EdenUI.plugins.Canvas2D.description = "Enables 3D rendering capabilities for drawing primitives and meshes using JS-Eden";
