import PropTypes from 'prop-types';
import React from 'react';
import { requireNativeComponent } from 'react-native';

class GSTPlayer extends React.Component {
  render() {
    return <GSTPlayer {...this.props} style={{ flex: 1 }}/>;
  }
}

GSTPlayer.propTypes = {
  
};

export default requireNativeComponent('RCTGSTPlayer', GSTPlayer);