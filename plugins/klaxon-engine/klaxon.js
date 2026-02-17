/**
 * JS-Eden Klaxon-Engine Plugin
 * Audio Engine used to create an audio instance within CONSTRUIT! and 
 * @class Scene Plugin
 */
EdenUI.plugins.Klaxon = function (edenUI, success) {
    edenUI.views["Klaxon"] = {dialog: this.createDialog, embedded: this.createEmbedded, title: "Klaxon", category: edenUI.viewCategories.visualization, holdsContent: true};
}

EdenUI.plugins.Canvas2D.title = "Klaxon";
EdenUI.plugins.Canvas2D.description = "Enables 3D rendering capabilities for drawing primitives and meshes using JS-Eden";
