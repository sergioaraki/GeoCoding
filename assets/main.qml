import bb.cascades 1.0
import bb.cascades.maps 1.0

Page {
    Container {
        layout: DockLayout {
        }
        Container {
            verticalAlignment: VerticalAlignment.Center
            leftPadding: 20
            layout: StackLayout {
            }
            Label {
                text: "La dirección es:"
            }
            Label {
                id: dir
                property string direccion: "Humboldt 2036, Buenos Aires, Argentina"
                text: "<html><span style='font-size:8pt;font-weight: bold;color:#0073bc;'>"+direccion+"</span></html>"
                gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                app.geoCoding(dir.direccion);
                                mapa.open();
                            }
                        }
                ]
            }
        }
    }
    attachedObjects: [
        Sheet {
            id: mapa
            objectName: "mapa"
            Page {
                titleBar: TitleBar {
                    acceptAction: ActionItem {
                        title: "Volver"
                        onTriggered: {
                            mapa.close();
                        }
                    }
                }
                Container {
                    layout: DockLayout {
                    }
                    Label {
                        id: errorGeo
                        objectName: "errorGeo"
                        text: "No se pudo geolocalizar la dirección"
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Center
                        visible: false
                    }
                    MapView {
                        id: mapView
                        objectName: "mapView"
                        visible: false
                        verticalAlignment: VerticalAlignment.Fill
                        horizontalAlignment: HorizontalAlignment.Fill
                    }
                    ActivityIndicator {
                        id: loading
                        objectName: "loading"
                        running: true
                        visible: running
                        verticalAlignment: VerticalAlignment.Fill
                        horizontalAlignment: HorizontalAlignment.Fill
                    }
                }
            }
        }
    ]
}
