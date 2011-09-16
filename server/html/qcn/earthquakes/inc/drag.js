/**
 * Drag.js:
 * This function is designed to be called from a mousedown event handler.
 * It registers temporary capturing event handlers for the mousemove and 
 * mouseup events that will follow and uses these handlers to "drag" the
 * specified document element. The first argument must be an absolutely
 * positioned document element. It may be the element that received the 
 * mousedown event or it may be some containing element. The second 
 * argument must be the event object for the mousedown event.
 **/
function beginDrag(elementToDrag, event) {
    // Figure out where the element currently is
    // The element must have left and top CSS properties in a style attribute
    // Also, we assume they are set using pixel units
    var x = parseInt(elementToDrag.style.left);
    var y = parseInt(elementToDrag.style.top);

    // Compute the distance between that point and the mouse-click
    // The nested moveHandler function below needs these values
    var deltaX = event.clientX - x;
    var deltaY = event.clientY - y;

    // Register the event handlers that will respond to the mousemove
    // and mouseup events that follow this mousedown event. Note that
    // these are registered as capturing event handlers on the document.
    // These event handlers remain active while the mouse button remains
    // pressed and are removed when the button is released.
    document.addEventListener("mousemove", moveHandler, true);
    document.addEventListener("mouseup", upHandler, true);

    // We've handled this event. Don't let anybody else see it.
    event.stopPropagation( );
    event.preventDefault( );

    /**
     * This is the handler that captures mousemove events when an element
     * is being dragged. It is responsible for moving the element.
     **/
    function moveHandler(event) {
        // Move the element to the current mouse position, adjusted as
        // necessary by the offset of the initial mouse-click
        elementToDrag.style.left = (event.clientX - deltaX) + "px";
        elementToDrag.style.top = (event.clientY - deltaY) + "px";

        // And don't let anyone else see this event
        event.stopPropagation( );
    }

    /**
     * This is the handler that captures the final mouseup event that
     * occurs at the end of a drag
     **/
    function upHandler(event) {
        // Unregister the capturing event handlers
        document.removeEventListener("mouseup", upHandler, true);
        document.removeEventListener("mousemove", moveHandler, true);
        // And don't let the event propagate any further
        event.stopPropagation( );
    }
}
