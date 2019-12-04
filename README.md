# lcui-router

[![Build Status](https://travis-ci.org/lc-soft/lcui-router.svg?branch=master)](https://travis-ci.org/lc-soft/lcui-router)
[![Coverage Status](https://coveralls.io/repos/github/lc-soft/lcui-router/badge.svg?branch=master)](https://coveralls.io/github/lc-soft/lcui-router?branch=master)

A router for control view switching and status in the [LCUI](https://github.com/lc-soft/LCUI) applications, it inspired by the [Vue Router](https://github.com/vuejs/vue-router/blob/dev/dist/vue-router.esm.js).

## Installation

**Windows:**

Install with [LCPkg](https://github.com/lc-soft/lcpkg):

```bash
lcpkg install github.com/lc-soft/lcui-router
```

**Linux:**

Download source code and copy it to your project directory.

## Usage

We recommend that you use [lcui-cli](https://github.com/lc-ui/lcui-cli) tool to manage the configuration and source code for the router, just follow these steps.

1. Create an LCUI application project:

    ``` bash
    lcui create myapp
    cd myapp
    ```

1. Overwrite the following code to the file `app/assets/views/app.xml`:

    ```xml
    <?xml version="1.0" encoding="UTF-8" ?>
    <lcui-app>
    <resource type="text/css" src="assets/stylesheets/app.css"/>
    <ui>
      <w>
        <textview>Hello App!</textview>
        <w>
          <!-- use router-link component for navigation. -->
          <!-- specify the link by passing the `to` prop. -->
          <router-link to="/foo">Go to Foo</router-link>
          <router-link to="/bar">Go to Bar</router-link>
        </w>
        <!-- route outlet -->
        <!-- component matched by the route will render here -->
        <router-view />
      </w>
    </ui>
    </lcui-app>
    ```

1. Generate widget source files:

    ```bash
    lcui generate widget foo
    lcui generate widget bar
    ```

1. Save following code as file `config/router.js`:

    ```js
    // config/router.js
    module.exports = [
      { path: '/foo', component: 'foo' },
      { path: '/bar', component: 'bar' }
    ]
    ```

1. Compile config file for router:

    ```bash
    lcui compile router
    ```

    You will see the following output:

    ```text
    output src/router.c
    output src/router.h
    update src/app.c
    ```

1. Run app:

    ``` bash
    lcpkg run start
    ```

## License

[MIT licensed](LICENSE).
