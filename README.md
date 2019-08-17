fig
===

Build configuration dialogs using simple JSON

Copyright (c) Grady O'Connell, 2019

Here's how it looks in action:

![fig](https://imgur.com/N1YSO94.png)

## Getting Started

First, get the dependencies:

```
git submodule update --init --recursive
```

Build with Qt qmake and then run make:

```
qmake
make
```

Fig looks for the settings files in the bin directory.

The file names should be:

- settings.json (this is where it saves, should contain "{}" at first)
- settings.schema.json (your schema here)

## Schema

### Categories

In the upper level of the json, put your categories, like this:

```
{
    "video":{
    },
    
    "audio":{
    }
}
```

### Names

Add the printable name of the category using ".name"

```
{
    "video":{
        ".name": "Video"
    },
        
    "audio":{
        ".name": "Audio"
    }
}
```

### Options

Add options beneath these categories.  You can use ".name" here too.

```
{
    "video":{
        ".name": "Video",
        "resolution": {
            ".name": "Screen Resolution"
        }
    }
}
```

The above results in a single text field for entering screen resolution.

### Defaults

Add default values with ".default":

```
{
    "video":{
        ".name": "Video",
        "resolution": {
            ".name": "Screen Resolution",
            ".default": "1920x1080"
        }
    }
}
```

Now the text field is filled in by default.

### Drop-down Lists

To creata a drop-down list with a set of values, use ".values":

```
{
    "video":{
        "AA": {
            ".name": "Anti-Aliasing",
            ".values": [ 1, 2, 4, 8, 16 ]
        }
    }
}

```

Sometimes you'll want the drop down to have different names than the value written into the config.
Here's an example of that:

```
{
    "options": {
        "mode": {
            ".name": "Output Mode",
            ".default": "synth",
            ".values": [ "synth", "midi" ],
            ".options": [ "Built-in Synthesizer", "MIDI" ]
        }
    }
}

```

### Checkbox

When values are true and false boolean values, the resulting widget is a single checkbox.

```
{
    "video":{
        "vsync": {
            ".name": "Vertical Sync",
            ".values": [ false, true ]
        }
    }
}
```

### Sliders

The following code creates a value slider:

```
{
    "audio": {
        "volume": {
            ".name": "Volume",
            ".step": 5,
            ".range": [0, 100],
            ".default": 100,
            ".suffix": "%"
        }
    }
}
```

### Application Info

To customize the dialog further, use these options:

```
{
    ".header": "App Name",
    ".icon": "logo.png",
    ".icon-width": 64,
    ".icon-height": 64,
    ".footer": "Visit our website at : <a href=\"http://website.com\">website.com</a>"
}
```


