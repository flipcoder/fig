fig
===

Build fully-functional configuration dialogs using simple JSON

Copyright (c) Grady O'Connell, 2019

Here's how it looks in action:

![fig](https://imgur.com/N1YSO94.png)

## Getting Started

After you've cloned the repo, get the dependencies:

```
git submodule update --init --recursive
```

Build with Qt qmake and then run make:

```
qmake
make
```

Fig looks for the schema file in the program's directory.  The default filename is "settings.schema.json".

You can load a different schema by passing it in:

```
./fig schema.json
```

The default save path is settings.json in the program's directory.
See *Schema / Application Info* on how to change where it saves.

## How it Works

Fig is a program that you can bundle with your application, meaning all you need to do is modify
settings.schema.json, and from there, your application can run fig as a configuration dialog,
and the generated settings file called settings.json with your user's
settings will be available to your application to make sense of (it's just json).

You may prefer your users to run fig separately from your main program, or
have it act as a launcher, calling your main program when the
user hits Ok and saves the settings (Read *Schema / Application Info* in the readme).
The other option is to call fig from within your application, and when the process closes,
have your application read settings.json to get the new information.

## Features

- Supports text fields, comboboxes, checkboxes, and sliders
- Supports string, int, double, and bool types
- Type-checking
- Restore defaults
- App logos
- Headers and footer text and links
- Launch an application upon saving

## Schema

### Categories

In the upper level of the settings.schema.json, put your categories, like this:

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

Now the text field is filled in by default.  The default determines the value's type.

Strings (default), ints, doubles are supported.

### Drop-down Lists

To create a drop-down list with a set of values, use ".values":

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

When the default value is a boolean, then the widget becomes a checkbox.

```
{
    "video":{
        "vsync": {
            ".name": "Vertical Sync",
            ".default": false
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

Suffix is optional.

### Application Settings

To customize the dialog further, use these options.  They are all optional.

```
{
    ".title": "App Settings",
    ".launch": "./my_app",
    ".header": "App Name",
    ".icon": "logo.png",
    ".icon-width": 64,
    ".icon-height": 64,
    ".footer": "Visit our website at : <a href=\"http://website.com\">website.com</a>",
    ".settings": "settings.json"
}
```

The launch instruction is the script that gets called when the user hits Ok (after it saves).

Your app should then read the resultant settings.json file and use a json parser to extract the settings.

## Return Codes

Fig gives different return codes based on how the dialog was closed.

- 0: User pressed "Ok" and settings were saved
- 1: Dialog window closed or "Cancel" button hit
- 2: Error: Most likely fig failed to load schema

## Questions, Bugs, Feature Requests?

If you have any questions or want to discuss this project, I'm always open for communication.

Post bugs and feature requests to github issues.  This is a new project so you might run into limitations or issues.

If you find this useful, give it a star on github. Thanks!

