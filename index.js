
'use strict';
import { NativeModules, requireNativeComponent, View } from 'react-native';

var iface = {
  name: 'GstPlayer',
    ...View.propTypes // include the default view properties
};
var GstPlayer = requireNativeComponent('GstPlayer', iface);

export default GstPlayer;
