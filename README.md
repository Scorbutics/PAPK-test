# ruby-vm

Initialize a fully compliant CRuby VM in a Ionic Capacitor context and allow to run scripts inside.

## Install

```bash
npm install ruby-vm
npx cap sync
```

## API

<docgen-index>

* [`create(...)`](#create)
* [`addListener('log', ...)`](#addlistenerlog-)
* [`addListener('logError', ...)`](#addlistenerlogerror-)
* [`execute(...)`](#execute)
* [`remove(...)`](#remove)
* [`removeAllListeners()`](#removealllisteners)
* [Interfaces](#interfaces)

</docgen-index>

<docgen-api>
<!--Update the source file JSDoc comments and rerun docgen to update the docs below-->

### create(...)

```typescript
create(options: { executionLocation: string; archiveLocation: string; }) => Promise<{ interpreter: number; }>
```

| Param         | Type                                                                 |
| ------------- | -------------------------------------------------------------------- |
| **`options`** | <code>{ executionLocation: string; archiveLocation: string; }</code> |

**Returns:** <code>Promise&lt;{ interpreter: number; }&gt;</code>

--------------------


### addListener('log', ...)

```typescript
addListener(eventName: 'log', listenerFunc: (data: { message: string; }) => void) => Promise<PluginListenerHandle>
```

| Param              | Type                                                 |
| ------------------ | ---------------------------------------------------- |
| **`eventName`**    | <code>'log'</code>                                   |
| **`listenerFunc`** | <code>(data: { message: string; }) =&gt; void</code> |

**Returns:** <code>Promise&lt;<a href="#pluginlistenerhandle">PluginListenerHandle</a>&gt;</code>

--------------------


### addListener('logError', ...)

```typescript
addListener(eventName: 'logError', listenerFunc: (data: { error: string; }) => void) => Promise<PluginListenerHandle>
```

| Param              | Type                                               |
| ------------------ | -------------------------------------------------- |
| **`eventName`**    | <code>'logError'</code>                            |
| **`listenerFunc`** | <code>(data: { error: string; }) =&gt; void</code> |

**Returns:** <code>Promise&lt;<a href="#pluginlistenerhandle">PluginListenerHandle</a>&gt;</code>

--------------------


### execute(...)

```typescript
execute(options: { interpreter: number; }) => Promise<{ result: number; }>
```

| Param         | Type                                  |
| ------------- | ------------------------------------- |
| **`options`** | <code>{ interpreter: number; }</code> |

**Returns:** <code>Promise&lt;{ result: number; }&gt;</code>

--------------------


### remove(...)

```typescript
remove(options: { interpreter: number; }) => Promise<{ index: number; } | void>
```

| Param         | Type                                  |
| ------------- | ------------------------------------- |
| **`options`** | <code>{ interpreter: number; }</code> |

**Returns:** <code>Promise&lt;void | { index: number; }&gt;</code>

--------------------


### removeAllListeners()

```typescript
removeAllListeners() => Promise<void>
```

--------------------


### Interfaces


#### PluginListenerHandle

| Prop         | Type                                      |
| ------------ | ----------------------------------------- |
| **`remove`** | <code>() =&gt; Promise&lt;void&gt;</code> |

</docgen-api>
