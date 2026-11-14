module.exports = [
  {
    type: 'heading',
    defaultValue: 'Heliochron'
  },
  {
    type: 'text',
    defaultValue:
      'A solar system on your wrist — Mars keeps the hours, Earth the minutes, Mercury the seconds. The Sun holds the center.'
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Time'
      },
      {
        type: 'select',
        messageKey: 'TimeFormat',
        defaultValue: '0',
        label: 'Hour format',
        options: [
          { label: 'System default', value: '0' },
          { label: '12-hour', value: '1' },
          { label: '24-hour', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'LeadingZero',
        label: 'Leading zero on hours',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDigital',
        label: 'Show digital time',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDate',
        label: 'Show date',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'HourlyVibe',
        label: 'Vibrate on the hour',
        defaultValue: false
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Solar System'
      },
      {
        type: 'select',
        messageKey: 'HourBody',
        defaultValue: '0',
        label: 'Hour planet',
        options: [
          { label: 'Mars (classic)', value: '0' },
          { label: 'Saturn (rings)', value: '1' },
          { label: 'Jupiter (bands)', value: '2' }
        ]
      },
      {
        type: 'select',
        messageKey: 'OrbitStyle',
        defaultValue: '0',
        label: 'Orbit shape',
        options: [
          { label: 'Circular', value: '0' },
          { label: 'Elliptical', value: '1' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'ShowSeconds',
        label: 'Mercury second hand',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowOrbits',
        label: 'Show orbit rings',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowStarfield',
        label: 'Starfield background',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowBatteryArc',
        label: 'Battery comet arc',
        defaultValue: false
      },
      {
        type: 'toggle',
        messageKey: 'Twinkle',
        label: 'Twinkling stars',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Look'
      },
      {
        type: 'select',
        messageKey: 'Theme',
        defaultValue: '0',
        label: 'Theme',
        options: [
          { label: 'Classic Cosmic', value: '0' },
          { label: 'Ember Void', value: '1' },
          { label: 'Ice Nebula', value: '2' },
          { label: 'Neon Drift', value: '3' }
        ]
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
