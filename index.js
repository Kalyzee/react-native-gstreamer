
'use strict';

import { PropTypes } from 'react';
import { NativeModules, requireNativeComponent, View } from 'react-native';

var iface = {
  name: 'GstPlayer',
  propTypes: {
    indeterminate: PropTypes.bool,
    ...View.propTypes // include the default view properties
  }
};
var GstPlayer = requireNativeComponent('GstPlayer', iface);

export default GstPlayer;
