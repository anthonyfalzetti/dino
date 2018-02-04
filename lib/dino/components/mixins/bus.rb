module Dino
  module Components
    module Mixins
      module Bus
        #
        # Behavior mixin for any component that tracks multiple other components
        # attached to it, matching Board, but without the full Board interface.
        # See OneWire::Bus class for an example.
        #
        def after_initialize(options={})
          super(options) if defined?(super)
          @components = []
        end

        attr_reader :components

        def add_component(component)
          @components << component
        end

        def remove_component(component)
          @components.delete(component)
        end
      end
    end
  end
end
