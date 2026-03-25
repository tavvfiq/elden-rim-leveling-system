// Minimal type shims so the UI can be edited
// in a workspace without installed node_modules.

declare module "react" {
  export type Dispatch<A> = (value: A) => void;
  export type SetStateAction<S> = S | ((prevState: S) => S);
  export function useState<S>(initialState: S): [S, Dispatch<SetStateAction<S>>];
  export function useEffect(effect: () => void | (() => void), deps?: any[]): void;
  export function useMemo<T>(factory: () => T, deps: any[]): T;
  const React: any;
  export default React;
}
declare module "react/jsx-runtime";

declare namespace JSX {
  interface IntrinsicElements {
    [elemName: string]: any;
  }
}

