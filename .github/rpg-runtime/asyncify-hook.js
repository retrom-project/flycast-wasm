// The host must wait for a suspended callMain to finish before querying the core.
Module["retromWhenAsyncifyDone"] = () =>
    Asyncify.currData ? Asyncify.whenDone() : Promise.resolve();
