package ag.boersego.bgjs;

/**
 * IV8GL
 * Interface for all v8-backed OpenGL views
 *
 * Copyright 2014 Kevin Read <me@kevin-read.com> and BörseGo AG (https://github.com/godmodelabs/ejecta-v8/)
 *
 **/

public interface IV8GL {

	public void requestRender();

    public void setInteractive(boolean b);

	public void unpause();

	public void pause();
	
    public void setRenderCallback (IV8GLViewOnRender listener);
	
	public interface IV8GLViewOnRender {
		public void renderStarted(int chartId);
		public void renderThreadClosed(int chartId);
	}
}
