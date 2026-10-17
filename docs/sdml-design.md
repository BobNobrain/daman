# SDML Design Document

## SDML

SDML stands for Structured Data Manipulation Language.
It is a language that serves as a simple tool to parse, process, view, and serialize structured data.
All you need to do is to describe the shape of your data, maybe annotate it with different attributes that tune the exact way the data is parsed/serialized/visualized, and then you can easily manipulate the resulting data via built-in tools and functions.

The core idea of SDML is the following: you declaratively describe your data structure, and the parsing/viewing/serialization come with it automatically.

## Goals

- Data descriptions are easy to read and understand even with no previous language knowledge;
- Data manipulation scripts are easy to read with a bit of preemptive learning required;
  - small ad-hoc scripts can be easy-to-write, and, if needed, automatically converted into more verbose but readable form;
- Data desriptions can be highly configurable, so that they can parse and serialize to any existing format – both text and binary;
- Parsing, serializing and scripts must be as fast as possible;
- Language can be compiled to C or some other system-level language to be used in other apps;
- Language can be compiled to some custom bytecode, ran by a custom lightweight easily embeddable VM;
- Language must have a type system that is simple and easy most of the time, but powerful and expressive when needed;
- Script execution must be sandboxed by default, to allow safe running of third-party modules and scripts

## Drawing Board

### Data Description Example

```
enum Status { Todo, InProgress, Done }
encoding Status <> string = match {
    Todo <> "TODO"
    InProgress <> "In Progress"
    Done <> "Done"
}

object TodoItemDone {
    done Date
}

object TodoItem {
    status Status
    title string, -> Markdown.H2, <- Markdown.AnyHeader
    text string
    created Date
    author string, matches ${^[A-Za-z_0-9]+$}

    .variant Todo {
        status Status.Todo
    }

    .variant InProgress {
        status Status.InProgress
        started Date
    }

    .variant Done {
        status Status.Done
        started Date
        finished Date
    }
}

encoding TodoItem <> JSON = auto

encoding TodoItem <> Markdown = Markdown.Document {
    Markdown.H2 allowLess=true allowMore=true .title
    Markdown.Table {
        Markdown.TableRow "Status" .status
        Markdown.TableRow "Author" .author
        Markdown.TableRow "Created" .created
        if ($ in TodoItem.InProgress || $ in TodoItem.Done) {
            Markdown.TableRow "Started" .started
        }
        if ($ in TodoItem.Done) {
            Markdown.TableRow "Finished" .finished
        }
    }

    Markdow.Section .text
}

encoding TodoItem <> Binary = Binary.Stream {}

```
